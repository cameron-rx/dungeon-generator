
#include "main.h"

//${CONFIG_BEGIN}
#define CFG_BINARY_FILES *.bin|*.dat
#define CFG_BRL_DATABUFFER_IMPLEMENTED 1
#define CFG_BRL_FILESTREAM_IMPLEMENTED 1
#define CFG_BRL_GAMETARGET_IMPLEMENTED 1
#define CFG_BRL_SOCKET_IMPLEMENTED 1
#define CFG_BRL_STREAM_IMPLEMENTED 1
#define CFG_BRL_THREAD_IMPLEMENTED 1
#define CFG_CONFIG debug
#define CFG_CPP_GC_MODE 1
#define CFG_GLFW_SWAP_INTERVAL -1
#define CFG_GLFW_USE_MINGW 1
#define CFG_GLFW_WINDOW_FULLSCREEN 0
#define CFG_GLFW_WINDOW_HEIGHT 720
#define CFG_GLFW_WINDOW_RESIZABLE 0
#define CFG_GLFW_WINDOW_TITLE Monkey Game
#define CFG_GLFW_WINDOW_WIDTH 1280
#define CFG_HOST winnt
#define CFG_IMAGE_FILES *.png|*.jpg
#define CFG_LANG cpp
#define CFG_MOJO_AUTO_SUSPEND_ENABLED 1
#define CFG_MOJO_DRIVER_IMPLEMENTED 1
#define CFG_MOJO_IMAGE_FILTERING_ENABLED 1
#define CFG_MUSIC_FILES *.wav|*.ogg
#define CFG_OPENGL_DEPTH_BUFFER_ENABLED 0
#define CFG_OPENGL_GLES20_ENABLED 0
#define CFG_SAFEMODE 0
#define CFG_SOUND_FILES *.wav|*.ogg
#define CFG_TARGET glfw
#define CFG_TEXT_FILES *.txt|*.xml|*.json
//${CONFIG_END}

//${TRANSCODE_BEGIN}

#include <wctype.h>
#include <locale.h>

// C++ Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.

//***** Monkey Types *****

typedef wchar_t Char;
template<class T> class Array;
class String;
class Object;

#if CFG_CPP_DOUBLE_PRECISION_FLOATS
typedef double Float;
#define FLOAT(X) X
#else
typedef float Float;
#define FLOAT(X) X##f
#endif

void dbg_error( const char *p );

#if !_MSC_VER
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

//***** GC Config *****

#define DEBUG_GC 0

// GC mode:
//
// 0 = disabled
// 1 = Incremental GC every OnWhatever
// 2 = Incremental GC every allocation
//
#ifndef CFG_CPP_GC_MODE
#define CFG_CPP_GC_MODE 1
#endif

//How many bytes alloced to trigger GC
//
#ifndef CFG_CPP_GC_TRIGGER
#define CFG_CPP_GC_TRIGGER 8*1024*1024
#endif

//GC_MODE 2 needs to track locals on a stack - this may need to be bumped if your app uses a LOT of locals, eg: is heavily recursive...
//
#ifndef CFG_CPP_GC_MAX_LOCALS
#define CFG_CPP_GC_MAX_LOCALS 8192
#endif

// ***** GC *****

#if _WIN32

int gc_micros(){
	static int f;
	static LARGE_INTEGER pcf;
	if( !f ){
		if( QueryPerformanceFrequency( &pcf ) && pcf.QuadPart>=1000000L ){
			pcf.QuadPart/=1000000L;
			f=1;
		}else{
			f=-1;
		}
	}
	if( f>0 ){
		LARGE_INTEGER pc;
		if( QueryPerformanceCounter( &pc ) ) return pc.QuadPart/pcf.QuadPart;
		f=-1;
	}
	return 0;// timeGetTime()*1000;
}

#elif __APPLE__

#include <mach/mach_time.h>

int gc_micros(){
	static int f;
	static mach_timebase_info_data_t timeInfo;
	if( !f ){
		mach_timebase_info( &timeInfo );
		timeInfo.denom*=1000L;
		f=1;
	}
	return mach_absolute_time()*timeInfo.numer/timeInfo.denom;
}

#else

int gc_micros(){
	return 0;
}

#endif

#define gc_mark_roots gc_mark

void gc_mark_roots();

struct gc_object;

gc_object *gc_object_alloc( int size );
void gc_object_free( gc_object *p );

struct gc_object{
	gc_object *succ;
	gc_object *pred;
	int flags;
	
	virtual ~gc_object(){
	}
	
	virtual void mark(){
	}
	
	void *operator new( size_t size ){
		return gc_object_alloc( size );
	}
	
	void operator delete( void *p ){
		gc_object_free( (gc_object*)p );
	}
};

gc_object gc_free_list;
gc_object gc_marked_list;
gc_object gc_unmarked_list;
gc_object gc_queued_list;	//doesn't really need to be doubly linked...

int gc_free_bytes;
int gc_marked_bytes;
int gc_alloced_bytes;
int gc_max_alloced_bytes;
int gc_new_bytes;
int gc_markbit=1;

gc_object *gc_cache[8];

int gc_ctor_nest;
gc_object *gc_locals[CFG_CPP_GC_MAX_LOCALS],**gc_locals_sp=gc_locals;

void gc_collect_all();
void gc_mark_queued( int n );

#define GC_CLEAR_LIST( LIST ) ((LIST).succ=(LIST).pred=&(LIST))

#define GC_LIST_IS_EMPTY( LIST ) ((LIST).succ==&(LIST))

#define GC_REMOVE_NODE( NODE ){\
(NODE)->pred->succ=(NODE)->succ;\
(NODE)->succ->pred=(NODE)->pred;}

#define GC_INSERT_NODE( NODE,SUCC ){\
(NODE)->pred=(SUCC)->pred;\
(NODE)->succ=(SUCC);\
(SUCC)->pred->succ=(NODE);\
(SUCC)->pred=(NODE);}

void gc_init1(){
	GC_CLEAR_LIST( gc_free_list );
	GC_CLEAR_LIST( gc_marked_list );
	GC_CLEAR_LIST( gc_unmarked_list);
	GC_CLEAR_LIST( gc_queued_list );
}

void gc_init2(){
	gc_mark_roots();
}

#if CFG_CPP_GC_MODE==2

struct gc_ctor{
	gc_ctor(){ ++gc_ctor_nest; }
	~gc_ctor(){ --gc_ctor_nest; }
};

struct gc_enter{
	gc_object **sp;
	gc_enter():sp(gc_locals_sp){
	}
	~gc_enter(){
	/*
		static int max_locals;
		int n=gc_locals_sp-gc_locals;
		if( n>max_locals ){
			max_locals=n;
			printf( "max_locals=%i\n",n );
		}
	*/
		gc_locals_sp=sp;
	}
};

#define GC_CTOR gc_ctor _c;
#define GC_ENTER gc_enter _e;

#else

struct gc_ctor{
};
struct gc_enter{
};

#define GC_CTOR
#define GC_ENTER

#endif

void gc_flush_free( int size ){

	int t=gc_free_bytes-size;
	if( t<0 ) t=0;
	
	while( gc_free_bytes>t ){
		gc_object *p=gc_free_list.succ;
		if( !p || p==&gc_free_list ){
//			printf("ERROR:p=%p gc_free_bytes=%i\n",p,gc_free_bytes);
//			fflush(stdout);
			gc_free_bytes=0;
			break;
		}
		GC_REMOVE_NODE(p);
		delete p;	//...to gc_free
	}
}

void *gc_ext_malloc( int size ){

	gc_new_bytes+=size;
	
	gc_flush_free( size );
	
	return malloc( size );
}

void gc_ext_malloced( int size ){

	gc_new_bytes+=size;
	
	gc_flush_free( size );
}

gc_object *gc_object_alloc( int size ){

	size=(size+7)&~7;
	
#if CFG_CPP_GC_MODE==1

	gc_new_bytes+=size;
	
#elif CFG_CPP_GC_MODE==2

	if( !gc_ctor_nest ){
#if DEBUG_GC
		int ms=gc_micros();
#endif
		if( gc_new_bytes+size>(CFG_CPP_GC_TRIGGER) ){
			gc_collect_all();
			gc_new_bytes=size;
		}else{
			gc_new_bytes+=size;
			gc_mark_queued( (long long)(gc_new_bytes)*(gc_alloced_bytes-gc_new_bytes)/(CFG_CPP_GC_TRIGGER)+gc_new_bytes );
		}
		
#if DEBUG_GC
		ms=gc_micros()-ms;
		if( ms>=100 ) {printf( "gc time:%i\n",ms );fflush( stdout );}
#endif
	}

#endif

	gc_flush_free( size );

	gc_object *p;
	if( size<64 && (p=gc_cache[size>>3]) ){
		gc_cache[size>>3]=p->succ;
	}else{
		p=(gc_object*)malloc( size );
	}
	
	p->flags=size|gc_markbit;
	GC_INSERT_NODE( p,&gc_unmarked_list );

	gc_alloced_bytes+=size;
	if( gc_alloced_bytes>gc_max_alloced_bytes ) gc_max_alloced_bytes=gc_alloced_bytes;
	
#if CFG_CPP_GC_MODE==2
	*gc_locals_sp++=p;
#endif

	return p;
}

void gc_object_free( gc_object *p ){

	int size=p->flags & ~7;
	gc_free_bytes-=size;
	
	if( size<64 ){
		p->succ=gc_cache[size>>3];
		gc_cache[size>>3]=p;
	}else{
		free( p );
	}
}

template<class T> void gc_mark( T *t ){

	gc_object *p=dynamic_cast<gc_object*>(t);
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_marked_list );
		gc_marked_bytes+=(p->flags & ~7);
		p->mark();
	}
}

template<class T> void gc_mark_q( T *t ){

	gc_object *p=dynamic_cast<gc_object*>(t);
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_queued_list );
	}
}

template<class T> T *gc_retain( T *t ){
#if CFG_CPP_GC_MODE==2
	*gc_locals_sp++=dynamic_cast<gc_object*>( t );
#endif	
	return t;
}

template<class T,class V> void gc_assign( T *&lhs,V *rhs ){
	gc_object *p=dynamic_cast<gc_object*>(rhs);
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_queued_list );
	}
	lhs=rhs;
}

void gc_mark_locals(){
	for( gc_object **pp=gc_locals;pp!=gc_locals_sp;++pp ){
		gc_object *p=*pp;
		if( p && (p->flags & 3)==gc_markbit ){
			p->flags^=1;
			GC_REMOVE_NODE( p );
			GC_INSERT_NODE( p,&gc_marked_list );
			gc_marked_bytes+=(p->flags & ~7);
			p->mark();
		}
	}
}

void gc_mark_queued( int n ){
	while( gc_marked_bytes<n && !GC_LIST_IS_EMPTY( gc_queued_list ) ){
		gc_object *p=gc_queued_list.succ;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_marked_list );
		gc_marked_bytes+=(p->flags & ~7);
		p->mark();
	}
}

//returns reclaimed bytes
int gc_sweep(){

	int reclaimed_bytes=gc_alloced_bytes-gc_marked_bytes;
	
	if( reclaimed_bytes ){
	
		//append unmarked list to end of free list
		gc_object *head=gc_unmarked_list.succ;
		gc_object *tail=gc_unmarked_list.pred;
		gc_object *succ=&gc_free_list;
		gc_object *pred=succ->pred;
		head->pred=pred;
		tail->succ=succ;
		pred->succ=head;
		succ->pred=tail;
		
		gc_free_bytes+=reclaimed_bytes;
	}
	
	//move marked to unmarked.
	gc_unmarked_list=gc_marked_list;
	gc_unmarked_list.succ->pred=gc_unmarked_list.pred->succ=&gc_unmarked_list;
	
	//clear marked.
	GC_CLEAR_LIST( gc_marked_list );
	
	//adjust sizes
	gc_alloced_bytes=gc_marked_bytes;
	gc_marked_bytes=0;
	gc_markbit^=1;
	
	return reclaimed_bytes;
}

void gc_collect_all(){

//	printf( "Mark locals\n" );fflush( stdout );
	gc_mark_locals();

//	printf( "Mark queued\n" );fflush( stdout );
	gc_mark_queued( 0x7fffffff );

//	printf( "sweep\n" );fflush( stdout );	
	gc_sweep();

//	printf( "Mark roots\n" );fflush( stdout );
	gc_mark_roots();

#if DEBUG_GC	
	printf( "gc collected:%i\n",reclaimed );fflush( stdout );
#endif
}

void gc_collect(){

	if( gc_locals_sp!=gc_locals ){
//		printf( "GC_LOCALS error\n" );fflush( stdout );
		gc_locals_sp=gc_locals;
	}
	
#if CFG_CPP_GC_MODE==1

#if DEBUG_GC
	int ms=gc_micros();
#endif

	if( gc_new_bytes>(CFG_CPP_GC_TRIGGER) ){
		gc_collect_all();
		gc_new_bytes=0;
	}else{
		gc_mark_queued( (long long)(gc_new_bytes)*(gc_alloced_bytes-gc_new_bytes)/(CFG_CPP_GC_TRIGGER)+gc_new_bytes );
	}

#if DEBUG_GC
	ms=gc_micros()-ms;
	if( ms>=100 ) {printf( "gc time:%i\n",ms );fflush( stdout );}
#endif

#endif

}

// ***** Array *****

template<class T> T *t_memcpy( T *dst,const T *src,int n ){
	memcpy( dst,src,n*sizeof(T) );
	return dst+n;
}

template<class T> T *t_memset( T *dst,int val,int n ){
	memset( dst,val,n*sizeof(T) );
	return dst+n;
}

template<class T> int t_memcmp( const T *x,const T *y,int n ){
	return memcmp( x,y,n*sizeof(T) );
}

template<class T> int t_strlen( const T *p ){
	const T *q=p++;
	while( *q++ ){}
	return q-p;
}

template<class T> T *t_create( int n,T *p ){
	t_memset( p,0,n );
	return p+n;
}

template<class T> T *t_create( int n,T *p,const T *q ){
	t_memcpy( p,q,n );
	return p+n;
}

template<class T> void t_destroy( int n,T *p ){
}

template<class T> void gc_mark_elements( int n,T *p ){
}

template<class T> void gc_mark_elements( int n,T **p ){
	for( int i=0;i<n;++i ) gc_mark( p[i] );
}

template<class T> class Array{
public:
	Array():rep( &nullRep ){
	}

	//Uses default...
//	Array( const Array<T> &t )...
	
	Array( int length ):rep( Rep::alloc( length ) ){
		t_create( rep->length,rep->data );
	}
	
	Array( const T *p,int length ):rep( Rep::alloc(length) ){
		t_create( rep->length,rep->data,p );
	}
	
	~Array(){
	}

	//Uses default...
//	Array &operator=( const Array &t )...
	
	int Length()const{ 
		return rep->length; 
	}
	
	T &At( int index ){
		if( index<0 || index>=rep->length ) dbg_error( "Array index out of range" );
		return rep->data[index]; 
	}
	
	const T &At( int index )const{
		if( index<0 || index>=rep->length ) dbg_error( "Array index out of range" );
		return rep->data[index]; 
	}
	
	T &operator[]( int index ){
		return rep->data[index]; 
	}

	const T &operator[]( int index )const{
		return rep->data[index]; 
	}
	
	Array Slice( int from,int term )const{
		int len=rep->length;
		if( from<0 ){ 
			from+=len;
			if( from<0 ) from=0;
		}else if( from>len ){
			from=len;
		}
		if( term<0 ){
			term+=len;
		}else if( term>len ){
			term=len;
		}
		if( term<=from ) return Array();
		return Array( rep->data+from,term-from );
	}

	Array Slice( int from )const{
		return Slice( from,rep->length );
	}
	
	Array Resize( int newlen )const{
		if( newlen<=0 ) return Array();
		int n=rep->length;
		if( newlen<n ) n=newlen;
		Rep *p=Rep::alloc( newlen );
		T *q=p->data;
		q=t_create( n,q,rep->data );
		q=t_create( (newlen-n),q );
		return Array( p );
	}
	
private:
	struct Rep : public gc_object{
		int length;
		T data[0];
		
		Rep():length(0){
			flags=3;
		}
		
		Rep( int length ):length(length){
		}
		
		~Rep(){
			t_destroy( length,data );
		}
		
		void mark(){
			gc_mark_elements( length,data );
		}
		
		static Rep *alloc( int length ){
			if( !length ) return &nullRep;
			void *p=gc_object_alloc( sizeof(Rep)+length*sizeof(T) );
			return ::new(p) Rep( length );
		}
		
	};
	Rep *rep;
	
	static Rep nullRep;
	
	template<class C> friend void gc_mark( Array<C> t );
	template<class C> friend void gc_mark_q( Array<C> t );
	template<class C> friend Array<C> gc_retain( Array<C> t );
	template<class C> friend void gc_assign( Array<C> &lhs,Array<C> rhs );
	template<class C> friend void gc_mark_elements( int n,Array<C> *p );
	
	Array( Rep *rep ):rep(rep){
	}
};

template<class T> typename Array<T>::Rep Array<T>::nullRep;

template<class T> Array<T> *t_create( int n,Array<T> *p ){
	for( int i=0;i<n;++i ) *p++=Array<T>();
	return p;
}

template<class T> Array<T> *t_create( int n,Array<T> *p,const Array<T> *q ){
	for( int i=0;i<n;++i ) *p++=*q++;
	return p;
}

template<class T> void gc_mark( Array<T> t ){
	gc_mark( t.rep );
}

template<class T> void gc_mark_q( Array<T> t ){
	gc_mark_q( t.rep );
}

template<class T> Array<T> gc_retain( Array<T> t ){
#if CFG_CPP_GC_MODE==2
	gc_retain( t.rep );
#endif
	return t;
}

template<class T> void gc_assign( Array<T> &lhs,Array<T> rhs ){
	gc_mark( rhs.rep );
	lhs=rhs;
}

template<class T> void gc_mark_elements( int n,Array<T> *p ){
	for( int i=0;i<n;++i ) gc_mark( p[i].rep );
}
		
// ***** String *****

static const char *_str_load_err;

class String{
public:
	String():rep( &nullRep ){
	}
	
	String( const String &t ):rep( t.rep ){
		rep->retain();
	}

	String( int n ){
		char buf[256];
		sprintf_s( buf,"%i",n );
		rep=Rep::alloc( t_strlen(buf) );
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
	}
	
	String( Float n ){
		char buf[256];
		
		//would rather use snprintf, but it's doing weird things in MingW.
		//
		sprintf_s( buf,"%.17lg",n );
		//
		char *p;
		for( p=buf;*p;++p ){
			if( *p=='.' || *p=='e' ) break;
		}
		if( !*p ){
			*p++='.';
			*p++='0';
			*p=0;
		}

		rep=Rep::alloc( t_strlen(buf) );
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
	}

	String( Char ch,int length ):rep( Rep::alloc(length) ){
		for( int i=0;i<length;++i ) rep->data[i]=ch;
	}

	String( const Char *p ):rep( Rep::alloc(t_strlen(p)) ){
		t_memcpy( rep->data,p,rep->length );
	}

	String( const Char *p,int length ):rep( Rep::alloc(length) ){
		t_memcpy( rep->data,p,rep->length );
	}
	
#if __OBJC__	
	String( NSString *nsstr ):rep( Rep::alloc([nsstr length]) ){
		unichar *buf=(unichar*)malloc( rep->length * sizeof(unichar) );
		[nsstr getCharacters:buf range:NSMakeRange(0,rep->length)];
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
		free( buf );
	}
#endif

#if __cplusplus_winrt
	String( Platform::String ^str ):rep( Rep::alloc(str->Length()) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=str->Data()[i];
	}
#endif

	~String(){
		rep->release();
	}
	
	template<class C> String( const C *p ):rep( Rep::alloc(t_strlen(p)) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=p[i];
	}
	
	template<class C> String( const C *p,int length ):rep( Rep::alloc(length) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=p[i];
	}
	
	int Length()const{
		return rep->length;
	}
	
	const Char *Data()const{
		return rep->data;
	}
	
	Char At( int index )const{
		if( index<0 || index>=rep->length ) dbg_error( "Character index out of range" );
		return rep->data[index]; 
	}
	
	Char operator[]( int index )const{
		return rep->data[index];
	}
	
	String &operator=( const String &t ){
		t.rep->retain();
		rep->release();
		rep=t.rep;
		return *this;
	}
	
	String &operator+=( const String &t ){
		return operator=( *this+t );
	}
	
	int Compare( const String &t )const{
		int n=rep->length<t.rep->length ? rep->length : t.rep->length;
		for( int i=0;i<n;++i ){
			if( int q=(int)(rep->data[i])-(int)(t.rep->data[i]) ) return q;
		}
		return rep->length-t.rep->length;
	}
	
	bool operator==( const String &t )const{
		return rep->length==t.rep->length && t_memcmp( rep->data,t.rep->data,rep->length )==0;
	}
	
	bool operator!=( const String &t )const{
		return rep->length!=t.rep->length || t_memcmp( rep->data,t.rep->data,rep->length )!=0;
	}
	
	bool operator<( const String &t )const{
		return Compare( t )<0;
	}
	
	bool operator<=( const String &t )const{
		return Compare( t )<=0;
	}
	
	bool operator>( const String &t )const{
		return Compare( t )>0;
	}
	
	bool operator>=( const String &t )const{
		return Compare( t )>=0;
	}
	
	String operator+( const String &t )const{
		if( !rep->length ) return t;
		if( !t.rep->length ) return *this;
		Rep *p=Rep::alloc( rep->length+t.rep->length );
		Char *q=p->data;
		q=t_memcpy( q,rep->data,rep->length );
		q=t_memcpy( q,t.rep->data,t.rep->length );
		return String( p );
	}
	
	int Find( String find,int start=0 )const{
		if( start<0 ) start=0;
		while( start+find.rep->length<=rep->length ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			++start;
		}
		return -1;
	}
	
	int FindLast( String find )const{
		int start=rep->length-find.rep->length;
		while( start>=0 ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			--start;
		}
		return -1;
	}
	
	int FindLast( String find,int start )const{
		if( start>rep->length-find.rep->length ) start=rep->length-find.rep->length;
		while( start>=0 ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			--start;
		}
		return -1;
	}
	
	String Trim()const{
		int i=0,i2=rep->length;
		while( i<i2 && rep->data[i]<=32 ) ++i;
		while( i2>i && rep->data[i2-1]<=32 ) --i2;
		if( i==0 && i2==rep->length ) return *this;
		return String( rep->data+i,i2-i );
	}

	Array<String> Split( String sep )const{
	
		if( !sep.rep->length ){
			Array<String> bits( rep->length );
			for( int i=0;i<rep->length;++i ){
				bits[i]=String( (Char)(*this)[i],1 );
			}
			return bits;
		}
		
		int i=0,i2,n=1;
		while( (i2=Find( sep,i ))!=-1 ){
			++n;
			i=i2+sep.rep->length;
		}
		Array<String> bits( n );
		if( n==1 ){
			bits[0]=*this;
			return bits;
		}
		i=0;n=0;
		while( (i2=Find( sep,i ))!=-1 ){
			bits[n++]=Slice( i,i2 );
			i=i2+sep.rep->length;
		}
		bits[n]=Slice( i );
		return bits;
	}

	String Join( Array<String> bits )const{
		if( bits.Length()==0 ) return String();
		if( bits.Length()==1 ) return bits[0];
		int newlen=rep->length * (bits.Length()-1);
		for( int i=0;i<bits.Length();++i ){
			newlen+=bits[i].rep->length;
		}
		Rep *p=Rep::alloc( newlen );
		Char *q=p->data;
		q=t_memcpy( q,bits[0].rep->data,bits[0].rep->length );
		for( int i=1;i<bits.Length();++i ){
			q=t_memcpy( q,rep->data,rep->length );
			q=t_memcpy( q,bits[i].rep->data,bits[i].rep->length );
		}
		return String( p );
	}

	String Replace( String find,String repl )const{
		int i=0,i2,newlen=0;
		while( (i2=Find( find,i ))!=-1 ){
			newlen+=(i2-i)+repl.rep->length;
			i=i2+find.rep->length;
		}
		if( !i ) return *this;
		newlen+=rep->length-i;
		Rep *p=Rep::alloc( newlen );
		Char *q=p->data;
		i=0;
		while( (i2=Find( find,i ))!=-1 ){
			q=t_memcpy( q,rep->data+i,i2-i );
			q=t_memcpy( q,repl.rep->data,repl.rep->length );
			i=i2+find.rep->length;
		}
		q=t_memcpy( q,rep->data+i,rep->length-i );
		return String( p );
	}

	String ToLower()const{
		for( int i=0;i<rep->length;++i ){
			Char t=towlower( rep->data[i] );
			if( t==rep->data[i] ) continue;
			Rep *p=Rep::alloc( rep->length );
			Char *q=p->data;
			t_memcpy( q,rep->data,i );
			for( q[i++]=t;i<rep->length;++i ){
				q[i]=towlower( rep->data[i] );
			}
			return String( p );
		}
		return *this;
	}

	String ToUpper()const{
		for( int i=0;i<rep->length;++i ){
			Char t=towupper( rep->data[i] );
			if( t==rep->data[i] ) continue;
			Rep *p=Rep::alloc( rep->length );
			Char *q=p->data;
			t_memcpy( q,rep->data,i );
			for( q[i++]=t;i<rep->length;++i ){
				q[i]=towupper( rep->data[i] );
			}
			return String( p );
		}
		return *this;
	}
	
	bool Contains( String sub )const{
		return Find( sub )!=-1;
	}

	bool StartsWith( String sub )const{
		return sub.rep->length<=rep->length && !t_memcmp( rep->data,sub.rep->data,sub.rep->length );
	}

	bool EndsWith( String sub )const{
		return sub.rep->length<=rep->length && !t_memcmp( rep->data+rep->length-sub.rep->length,sub.rep->data,sub.rep->length );
	}
	
	String Slice( int from,int term )const{
		int len=rep->length;
		if( from<0 ){
			from+=len;
			if( from<0 ) from=0;
		}else if( from>len ){
			from=len;
		}
		if( term<0 ){
			term+=len;
		}else if( term>len ){
			term=len;
		}
		if( term<from ) return String();
		if( from==0 && term==len ) return *this;
		return String( rep->data+from,term-from );
	}

	String Slice( int from )const{
		return Slice( from,rep->length );
	}
	
	Array<int> ToChars()const{
		Array<int> chars( rep->length );
		for( int i=0;i<rep->length;++i ) chars[i]=rep->data[i];
		return chars;
	}
	
	int ToInt()const{
		char buf[64];
		return atoi( ToCString<char>( buf,sizeof(buf) ) );
	}
	
	Float ToFloat()const{
		char buf[256];
		return atof( ToCString<char>( buf,sizeof(buf) ) );
	}

	template<class C> class CString{
		struct Rep{
			int refs;
			C data[1];
		};
		Rep *_rep;
		static Rep _nul;
	public:
		template<class T> CString( const T *data,int length ){
			_rep=(Rep*)malloc( length*sizeof(C)+sizeof(Rep) );
			_rep->refs=1;
			_rep->data[length]=0;
			for( int i=0;i<length;++i ){
				_rep->data[i]=(C)data[i];
			}
		}
		CString():_rep( new Rep ){
			_rep->refs=1;
		}
		CString( const CString &c ):_rep(c._rep){
			++_rep->refs;
		}
		~CString(){
			if( !--_rep->refs ) free( _rep );
		}
		CString &operator=( const CString &c ){
			++c._rep->refs;
			if( !--_rep->refs ) free( _rep );
			_rep=c._rep;
			return *this;
		}
		operator const C*()const{ 
			return _rep->data;
		}
	};
	
	template<class C> CString<C> ToCString()const{
		return CString<C>( rep->data,rep->length );
	}

	template<class C> C *ToCString( C *p,int length )const{
		if( --length>rep->length ) length=rep->length;
		for( int i=0;i<length;++i ) p[i]=rep->data[i];
		p[length]=0;
		return p;
	}
	
#if __OBJC__	
	NSString *ToNSString()const{
		return [NSString stringWithCharacters:ToCString<unichar>() length:rep->length];
	}
#endif

#if __cplusplus_winrt
	Platform::String ^ToWinRTString()const{
		return ref new Platform::String( rep->data,rep->length );
	}
#endif

	bool Save( FILE *fp ){
		std::vector<unsigned char> buf;
		Save( buf );
		return buf.size() ? fwrite( &buf[0],1,buf.size(),fp )==buf.size() : true;
	}
	
	void Save( std::vector<unsigned char> &buf ){
	
		Char *p=rep->data;
		Char *e=p+rep->length;
		
		while( p<e ){
			Char c=*p++;
			if( c<0x80 ){
				buf.push_back( c );
			}else if( c<0x800 ){
				buf.push_back( 0xc0 | (c>>6) );
				buf.push_back( 0x80 | (c & 0x3f) );
			}else{
				buf.push_back( 0xe0 | (c>>12) );
				buf.push_back( 0x80 | ((c>>6) & 0x3f) );
				buf.push_back( 0x80 | (c & 0x3f) );
			}
		}
	}
	
	static String FromChars( Array<int> chars ){
		int n=chars.Length();
		Rep *p=Rep::alloc( n );
		for( int i=0;i<n;++i ){
			p->data[i]=chars[i];
		}
		return String( p );
	}

	static String Load( FILE *fp ){
		unsigned char tmp[4096];
		std::vector<unsigned char> buf;
		for(;;){
			int n=fread( tmp,1,4096,fp );
			if( n>0 ) buf.insert( buf.end(),tmp,tmp+n );
			if( n!=4096 ) break;
		}
		return buf.size() ? String::Load( &buf[0],buf.size() ) : String();
	}
	
	static String Load( unsigned char *p,int n ){
	
		_str_load_err=0;
		
		unsigned char *e=p+n;
		std::vector<Char> chars;
		
		int t0=n>0 ? p[0] : -1;
		int t1=n>1 ? p[1] : -1;

		if( t0==0xfe && t1==0xff ){
			p+=2;
			while( p<e-1 ){
				int c=*p++;
				chars.push_back( (c<<8)|*p++ );
			}
		}else if( t0==0xff && t1==0xfe ){
			p+=2;
			while( p<e-1 ){
				int c=*p++;
				chars.push_back( (*p++<<8)|c );
			}
		}else{
			int t2=n>2 ? p[2] : -1;
			if( t0==0xef && t1==0xbb && t2==0xbf ) p+=3;
			unsigned char *q=p;
			bool fail=false;
			while( p<e ){
				unsigned int c=*p++;
				if( c & 0x80 ){
					if( (c & 0xe0)==0xc0 ){
						if( p>=e || (p[0] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x1f)<<6) | (p[0] & 0x3f);
						p+=1;
					}else if( (c & 0xf0)==0xe0 ){
						if( p+1>=e || (p[0] & 0xc0)!=0x80 || (p[1] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x0f)<<12) | ((p[0] & 0x3f)<<6) | (p[1] & 0x3f);
						p+=2;
					}else{
						fail=true;
						break;
					}
				}
				chars.push_back( c );
			}
			if( fail ){
				_str_load_err="Invalid UTF-8";
				return String( q,n );
			}
		}
		return chars.size() ? String( &chars[0],chars.size() ) : String();
	}

private:
	
	struct Rep{
		int refs;
		int length;
		Char data[0];
		
		Rep():refs(1),length(0){
		}
		
		Rep( int length ):refs(1),length(length){
		}
		
		void retain(){
			++refs;
		}
		
		void release(){
			if( --refs || !length ) return;
			free( this );
		}

		static Rep *alloc( int length ){
			if( !length ) return &nullRep;
			void *p=malloc( sizeof(Rep)+length*sizeof(Char) );
			return new(p) Rep( length );
		}
	};
	Rep *rep;
	
	static Rep nullRep;
	
	String( Rep *rep ):rep(rep){
	}
};

String::Rep String::nullRep;

String *t_create( int n,String *p ){
	for( int i=0;i<n;++i ) new( &p[i] ) String();
	return p+n;
}

String *t_create( int n,String *p,const String *q ){
	for( int i=0;i<n;++i ) new( &p[i] ) String( q[i] );
	return p+n;
}

void t_destroy( int n,String *p ){
	for( int i=0;i<n;++i ) p[i].~String();
}

// ***** Object *****

String dbg_stacktrace();

class Object : public gc_object{
public:
	virtual bool Equals( Object *obj ){
		return this==obj;
	}
	
	virtual int Compare( Object *obj ){
		return (char*)this-(char*)obj;
	}
	
	virtual String debug(){
		return "+Object\n";
	}
};

class ThrowableObject : public Object{
#ifndef NDEBUG
public:
	String stackTrace;
	ThrowableObject():stackTrace( dbg_stacktrace() ){}
#endif
};

struct gc_interface{
	virtual ~gc_interface(){}
};

//***** Debugger *****

//#define Error bbError
//#define Print bbPrint

int bbPrint( String t );

#define dbg_stream stderr

#if _MSC_VER
#define dbg_typeof decltype
#else
#define dbg_typeof __typeof__
#endif 

struct dbg_func;
struct dbg_var_type;

static int dbg_suspend;
static int dbg_stepmode;

const char *dbg_info;
String dbg_exstack;

static void *dbg_var_buf[65536*3];
static void **dbg_var_ptr=dbg_var_buf;

static dbg_func *dbg_func_buf[1024];
static dbg_func **dbg_func_ptr=dbg_func_buf;

String dbg_type( bool *p ){
	return "Bool";
}

String dbg_type( int *p ){
	return "Int";
}

String dbg_type( Float *p ){
	return "Float";
}

String dbg_type( String *p ){
	return "String";
}

template<class T> String dbg_type( T *p ){
	return "Object";
}

template<class T> String dbg_type( Array<T> *p ){
	return dbg_type( &(*p)[0] )+"[]";
}

String dbg_value( bool *p ){
	return *p ? "True" : "False";
}

String dbg_value( int *p ){
	return String( *p );
}

String dbg_value( Float *p ){
	return String( *p );
}

String dbg_value( String *p ){
	String t=*p;
	if( t.Length()>100 ) t=t.Slice( 0,100 )+"...";
	t=t.Replace( "\"","~q" );
	t=t.Replace( "\t","~t" );
	t=t.Replace( "\n","~n" );
	t=t.Replace( "\r","~r" );
	return String("\"")+t+"\"";
}

template<class T> String dbg_value( T *t ){
	Object *p=dynamic_cast<Object*>( *t );
	char buf[64];
	sprintf_s( buf,"%p",p );
	return String("@") + (buf[0]=='0' && buf[1]=='x' ? buf+2 : buf );
}

template<class T> String dbg_value( Array<T> *p ){
	String t="[";
	int n=(*p).Length();
	for( int i=0;i<n;++i ){
		if( i ) t+=",";
		t+=dbg_value( &(*p)[i] );
	}
	return t+"]";
}

template<class T> String dbg_decl( const char *id,T *ptr ){
	return String( id )+":"+dbg_type(ptr)+"="+dbg_value(ptr)+"\n";
}

struct dbg_var_type{
	virtual String type( void *p )=0;
	virtual String value( void *p )=0;
};

template<class T> struct dbg_var_type_t : public dbg_var_type{

	String type( void *p ){
		return dbg_type( (T*)p );
	}
	
	String value( void *p ){
		return dbg_value( (T*)p );
	}
	
	static dbg_var_type_t<T> info;
};
template<class T> dbg_var_type_t<T> dbg_var_type_t<T>::info;

struct dbg_blk{
	void **var_ptr;
	
	dbg_blk():var_ptr(dbg_var_ptr){
		if( dbg_stepmode=='l' ) --dbg_suspend;
	}
	
	~dbg_blk(){
		if( dbg_stepmode=='l' ) ++dbg_suspend;
		dbg_var_ptr=var_ptr;
	}
};

struct dbg_func : public dbg_blk{
	const char *id;
	const char *info;

	dbg_func( const char *p ):id(p),info(dbg_info){
		*dbg_func_ptr++=this;
		if( dbg_stepmode=='s' ) --dbg_suspend;
	}
	
	~dbg_func(){
		if( dbg_stepmode=='s' ) ++dbg_suspend;
		--dbg_func_ptr;
		dbg_info=info;
	}
};

int dbg_print( String t ){
	static char *buf;
	static int len;
	int n=t.Length();
	if( n+100>len ){
		len=n+100;
		free( buf );
		buf=(char*)malloc( len );
	}
	buf[n]='\n';
	for( int i=0;i<n;++i ) buf[i]=t[i];
	fwrite( buf,n+1,1,dbg_stream );
	fflush( dbg_stream );
	return 0;
}

void dbg_callstack(){

	void **var_ptr=dbg_var_buf;
	dbg_func **func_ptr=dbg_func_buf;
	
	while( var_ptr!=dbg_var_ptr ){
		while( func_ptr!=dbg_func_ptr && var_ptr==(*func_ptr)->var_ptr ){
			const char *id=(*func_ptr++)->id;
			const char *info=func_ptr!=dbg_func_ptr ? (*func_ptr)->info : dbg_info;
			fprintf( dbg_stream,"+%s;%s\n",id,info );
		}
		void *vp=*var_ptr++;
		const char *nm=(const char*)*var_ptr++;
		dbg_var_type *ty=(dbg_var_type*)*var_ptr++;
		dbg_print( String(nm)+":"+ty->type(vp)+"="+ty->value(vp) );
	}
	while( func_ptr!=dbg_func_ptr ){
		const char *id=(*func_ptr++)->id;
		const char *info=func_ptr!=dbg_func_ptr ? (*func_ptr)->info : dbg_info;
		fprintf( dbg_stream,"+%s;%s\n",id,info );
	}
}

String dbg_stacktrace(){
	if( !dbg_info || !dbg_info[0] ) return "";
	String str=String( dbg_info )+"\n";
	dbg_func **func_ptr=dbg_func_ptr;
	if( func_ptr==dbg_func_buf ) return str;
	while( --func_ptr!=dbg_func_buf ){
		str+=String( (*func_ptr)->info )+"\n";
	}
	return str;
}

void dbg_throw( const char *err ){
	dbg_exstack=dbg_stacktrace();
	throw err;
}

void dbg_stop(){

#if TARGET_OS_IPHONE
	dbg_throw( "STOP" );
#endif

	fprintf( dbg_stream,"{{~~%s~~}}\n",dbg_info );
	dbg_callstack();
	dbg_print( "" );
	
	for(;;){

		char buf[256];
		char *e=fgets( buf,256,stdin );
		if( !e ) exit( -1 );
		
		e=strchr( buf,'\n' );
		if( !e ) exit( -1 );
		
		*e=0;
		
		Object *p;
		
		switch( buf[0] ){
		case '?':
			break;
		case 'r':	//run
			dbg_suspend=0;		
			dbg_stepmode=0;
			return;
		case 's':	//step
			dbg_suspend=1;
			dbg_stepmode='s';
			return;
		case 'e':	//enter func
			dbg_suspend=1;
			dbg_stepmode='e';
			return;
		case 'l':	//leave block
			dbg_suspend=0;
			dbg_stepmode='l';
			return;
		case '@':	//dump object
			p=0;
			sscanf_s( buf+1,"%p",&p );
			if( p ){
				dbg_print( p->debug() );
			}else{
				dbg_print( "" );
			}
			break;
		case 'q':	//quit!
			exit( 0 );
			break;			
		default:
			printf( "????? %s ?????",buf );fflush( stdout );
			exit( -1 );
		}
	}
}

void dbg_error( const char *err ){

#if TARGET_OS_IPHONE
	dbg_throw( err );
#endif

	for(;;){
		bbPrint( String("Monkey Runtime Error : ")+err );
		bbPrint( dbg_stacktrace() );
		dbg_stop();
	}
}

#define DBG_INFO(X) dbg_info=(X);if( dbg_suspend>0 ) dbg_stop();

#define DBG_ENTER(P) dbg_func _dbg_func(P);

#define DBG_BLOCK() dbg_blk _dbg_blk;

#define DBG_GLOBAL( ID,NAME )	//TODO!

#define DBG_LOCAL( ID,NAME )\
*dbg_var_ptr++=&ID;\
*dbg_var_ptr++=(void*)NAME;\
*dbg_var_ptr++=&dbg_var_type_t<dbg_typeof(ID)>::info;

//**** main ****

int argc;
const char **argv;

Float D2R=0.017453292519943295f;
Float R2D=57.29577951308232f;

int bbPrint( String t ){

	static std::vector<unsigned char> buf;
	buf.clear();
	t.Save( buf );
	buf.push_back( '\n' );
	buf.push_back( 0 );
	
#if __cplusplus_winrt	//winrt?

#if CFG_WINRT_PRINT_ENABLED
	OutputDebugStringA( (const char*)&buf[0] );
#endif

#elif _WIN32			//windows?

	fputs( (const char*)&buf[0],stdout );
	fflush( stdout );

#elif __APPLE__			//macos/ios?

	fputs( (const char*)&buf[0],stdout );
	fflush( stdout );
	
#elif __linux			//linux?

#if CFG_ANDROID_NDK_PRINT_ENABLED
	LOGI( (const char*)&buf[0] );
#else
	fputs( (const char*)&buf[0],stdout );
	fflush( stdout );
#endif

#endif

	return 0;
}

class BBExitApp{
};

int bbError( String err ){
	if( !err.Length() ){
#if __cplusplus_winrt
		throw BBExitApp();
#else
		exit( 0 );
#endif
	}
	dbg_error( err.ToCString<char>() );
	return 0;
}

int bbDebugLog( String t ){
	bbPrint( t );
	return 0;
}

int bbDebugStop(){
	dbg_stop();
	return 0;
}

int bbInit();
int bbMain();

#if _MSC_VER

static void _cdecl seTranslator( unsigned int ex,EXCEPTION_POINTERS *p ){

	switch( ex ){
	case EXCEPTION_ACCESS_VIOLATION:dbg_error( "Memory access violation" );
	case EXCEPTION_ILLEGAL_INSTRUCTION:dbg_error( "Illegal instruction" );
	case EXCEPTION_INT_DIVIDE_BY_ZERO:dbg_error( "Integer divide by zero" );
	case EXCEPTION_STACK_OVERFLOW:dbg_error( "Stack overflow" );
	}
	dbg_error( "Unknown exception" );
}

#else

void sighandler( int sig  ){
	switch( sig ){
	case SIGSEGV:dbg_error( "Memory access violation" );
	case SIGILL:dbg_error( "Illegal instruction" );
	case SIGFPE:dbg_error( "Floating point exception" );
#if !_WIN32
	case SIGBUS:dbg_error( "Bus error" );
#endif	
	}
	dbg_error( "Unknown signal" );
}

#endif

//entry point call by target main()...
//
int bb_std_main( int argc,const char **argv ){

	::argc=argc;
	::argv=argv;
	
#if _MSC_VER

	_set_se_translator( seTranslator );

#else
	
	signal( SIGSEGV,sighandler );
	signal( SIGILL,sighandler );
	signal( SIGFPE,sighandler );
#if !_WIN32
	signal( SIGBUS,sighandler );
#endif

#endif

	if( !setlocale( LC_CTYPE,"en_US.UTF-8" ) ){
		setlocale( LC_CTYPE,"" );
	}

	gc_init1();

	bbInit();
	
	gc_init2();

	bbMain();

	return 0;
}


//***** game.h *****

struct BBGameEvent{
	enum{
		None=0,
		KeyDown=1,KeyUp=2,KeyChar=3,
		MouseDown=4,MouseUp=5,MouseMove=6,
		TouchDown=7,TouchUp=8,TouchMove=9,
		MotionAccel=10
	};
};

class BBGameDelegate : public Object{
public:
	virtual void StartGame(){}
	virtual void SuspendGame(){}
	virtual void ResumeGame(){}
	virtual void UpdateGame(){}
	virtual void RenderGame(){}
	virtual void KeyEvent( int event,int data ){}
	virtual void MouseEvent( int event,int data,float x,float y ){}
	virtual void TouchEvent( int event,int data,float x,float y ){}
	virtual void MotionEvent( int event,int data,float x,float y,float z ){}
	virtual void DiscardGraphics(){}
};

struct BBDisplayMode : public Object{
	int width;
	int height;
	int format;
	int hertz;
	int flags;
	BBDisplayMode( int width=0,int height=0,int format=0,int hertz=0,int flags=0 ):width(width),height(height),format(format),hertz(hertz),flags(flags){}
};

class BBGame{
public:
	BBGame();
	virtual ~BBGame(){}
	
	// ***** Extern *****
	static BBGame *Game(){ return _game; }
	
	virtual void SetDelegate( BBGameDelegate *delegate );
	virtual BBGameDelegate *Delegate(){ return _delegate; }
	
	virtual void SetKeyboardEnabled( bool enabled );
	virtual bool KeyboardEnabled();
	
	virtual void SetUpdateRate( int updateRate );
	virtual int UpdateRate();
	
	virtual bool Started(){ return _started; }
	virtual bool Suspended(){ return _suspended; }
	
	virtual int Millisecs();
	virtual void GetDate( Array<int> date );
	virtual int SaveState( String state );
	virtual String LoadState();
	virtual String LoadString( String path );
	virtual bool PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons );
	virtual void OpenUrl( String url );
	virtual void SetMouseVisible( bool visible );
	
	virtual int GetDeviceWidth(){ return 0; }
	virtual int GetDeviceHeight(){ return 0; }
	virtual void SetDeviceWindow( int width,int height,int flags ){}
	virtual Array<BBDisplayMode*> GetDisplayModes(){ return Array<BBDisplayMode*>(); }
	virtual BBDisplayMode *GetDesktopMode(){ return 0; }
	virtual void SetSwapInterval( int interval ){}

	// ***** Native *****	
	virtual String PathToFilePath( String path );
	virtual FILE *OpenFile( String path,String mode );
	virtual unsigned char *LoadData( String path,int *plength );
	virtual unsigned char *LoadImageData( String path,int *width,int *height,int *depth ){ return 0; }
	virtual unsigned char *LoadAudioData( String path,int *length,int *channels,int *format,int *hertz ){ return 0; }
	
	//***** Internal *****
	virtual void Die( ThrowableObject *ex );
	virtual void gc_collect();
	virtual void StartGame();
	virtual void SuspendGame();
	virtual void ResumeGame();
	virtual void UpdateGame();
	virtual void RenderGame();
	virtual void KeyEvent( int ev,int data );
	virtual void MouseEvent( int ev,int data,float x,float y );
	virtual void TouchEvent( int ev,int data,float x,float y );
	virtual void MotionEvent( int ev,int data,float x,float y,float z );
	virtual void DiscardGraphics();
	
protected:

	static BBGame *_game;

	BBGameDelegate *_delegate;
	bool _keyboardEnabled;
	int _updateRate;
	bool _started;
	bool _suspended;
};

//***** game.cpp *****

BBGame *BBGame::_game;

BBGame::BBGame():
_delegate( 0 ),
_keyboardEnabled( false ),
_updateRate( 0 ),
_started( false ),
_suspended( false ){
	_game=this;
}

void BBGame::SetDelegate( BBGameDelegate *delegate ){
	_delegate=delegate;
}

void BBGame::SetKeyboardEnabled( bool enabled ){
	_keyboardEnabled=enabled;
}

bool BBGame::KeyboardEnabled(){
	return _keyboardEnabled;
}

void BBGame::SetUpdateRate( int updateRate ){
	_updateRate=updateRate;
}

int BBGame::UpdateRate(){
	return _updateRate;
}

int BBGame::Millisecs(){
	return 0;
}

void BBGame::GetDate( Array<int> date ){
	int n=date.Length();
	if( n>0 ){
		time_t t=time( 0 );
		
#if _MSC_VER
		struct tm tii;
		struct tm *ti=&tii;
		localtime_s( ti,&t );
#else
		struct tm *ti=localtime( &t );
#endif

		date[0]=ti->tm_year+1900;
		if( n>1 ){ 
			date[1]=ti->tm_mon+1;
			if( n>2 ){
				date[2]=ti->tm_mday;
				if( n>3 ){
					date[3]=ti->tm_hour;
					if( n>4 ){
						date[4]=ti->tm_min;
						if( n>5 ){
							date[5]=ti->tm_sec;
							if( n>6 ){
								date[6]=0;
							}
						}
					}
				}
			}
		}
	}
}

int BBGame::SaveState( String state ){
	if( FILE *f=OpenFile( "./.monkeystate","wb" ) ){
		bool ok=state.Save( f );
		fclose( f );
		return ok ? 0 : -2;
	}
	return -1;
}

String BBGame::LoadState(){
	if( FILE *f=OpenFile( "./.monkeystate","rb" ) ){
		String str=String::Load( f );
		fclose( f );
		return str;
	}
	return "";
}

String BBGame::LoadString( String path ){
	if( FILE *fp=OpenFile( path,"rb" ) ){
		String str=String::Load( fp );
		fclose( fp );
		return str;
	}
	return "";
}

bool BBGame::PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons ){
	return false;
}

void BBGame::OpenUrl( String url ){
}

void BBGame::SetMouseVisible( bool visible ){
}

//***** C++ Game *****

String BBGame::PathToFilePath( String path ){
	return path;
}

FILE *BBGame::OpenFile( String path,String mode ){
	path=PathToFilePath( path );
	if( path=="" ) return 0;
	
#if __cplusplus_winrt
	path=path.Replace( "/","\\" );
	FILE *f;
	if( _wfopen_s( &f,path.ToCString<wchar_t>(),mode.ToCString<wchar_t>() ) ) return 0;
	return f;
#elif _WIN32
	return _wfopen( path.ToCString<wchar_t>(),mode.ToCString<wchar_t>() );
#else
	return fopen( path.ToCString<char>(),mode.ToCString<char>() );
#endif
}

unsigned char *BBGame::LoadData( String path,int *plength ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;

	const int BUF_SZ=4096;
	std::vector<void*> tmps;
	int length=0;
	
	for(;;){
		void *p=malloc( BUF_SZ );
		int n=fread( p,1,BUF_SZ,f );
		tmps.push_back( p );
		length+=n;
		if( n!=BUF_SZ ) break;
	}
	fclose( f );
	
	unsigned char *data=(unsigned char*)malloc( length );
	unsigned char *p=data;
	
	int sz=length;
	for( int i=0;i<tmps.size();++i ){
		int n=sz>BUF_SZ ? BUF_SZ : sz;
		memcpy( p,tmps[i],n );
		free( tmps[i] );
		sz-=n;
		p+=n;
	}
	
	*plength=length;
	
	gc_ext_malloced( length );
	
	return data;
}

//***** INTERNAL *****

void BBGame::Die( ThrowableObject *ex ){
	bbPrint( "Monkey Runtime Error : Uncaught Monkey Exception" );
#ifndef NDEBUG
	bbPrint( ex->stackTrace );
#endif
	exit( -1 );
}

void BBGame::gc_collect(){
	gc_mark( _delegate );
	::gc_collect();
}

void BBGame::StartGame(){

	if( _started ) return;
	_started=true;
	
	try{
		_delegate->StartGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::SuspendGame(){

	if( !_started || _suspended ) return;
	_suspended=true;
	
	try{
		_delegate->SuspendGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::ResumeGame(){

	if( !_started || !_suspended ) return;
	_suspended=false;
	
	try{
		_delegate->ResumeGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::UpdateGame(){

	if( !_started || _suspended ) return;
	
	try{
		_delegate->UpdateGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::RenderGame(){

	if( !_started ) return;
	
	try{
		_delegate->RenderGame();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::KeyEvent( int ev,int data ){

	if( !_started ) return;
	
	try{
		_delegate->KeyEvent( ev,data );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::MouseEvent( int ev,int data,float x,float y ){

	if( !_started ) return;
	
	try{
		_delegate->MouseEvent( ev,data,x,y );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::TouchEvent( int ev,int data,float x,float y ){

	if( !_started ) return;
	
	try{
		_delegate->TouchEvent( ev,data,x,y );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::MotionEvent( int ev,int data,float x,float y,float z ){

	if( !_started ) return;
	
	try{
		_delegate->MotionEvent( ev,data,x,y,z );
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}

void BBGame::DiscardGraphics(){

	if( !_started ) return;
	
	try{
		_delegate->DiscardGraphics();
	}catch( ThrowableObject *ex ){
		Die( ex );
	}
	gc_collect();
}


//***** wavloader.h *****
//
unsigned char *LoadWAV( FILE *f,int *length,int *channels,int *format,int *hertz );

//***** wavloader.cpp *****
//
static const char *readTag( FILE *f ){
	static char buf[8];
	if( fread( buf,4,1,f )!=1 ) return "";
	buf[4]=0;
	return buf;
}

static int readInt( FILE *f ){
	unsigned char buf[4];
	if( fread( buf,4,1,f )!=1 ) return -1;
	return (buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
}

static int readShort( FILE *f ){
	unsigned char buf[2];
	if( fread( buf,2,1,f )!=1 ) return -1;
	return (buf[1]<<8) | buf[0];
}

static void skipBytes( int n,FILE *f ){
	char *p=(char*)malloc( n );
	fread( p,n,1,f );
	free( p );
}

unsigned char *LoadWAV( FILE *f,int *plength,int *pchannels,int *pformat,int *phertz ){
	if( !strcmp( readTag( f ),"RIFF" ) ){
		int len=readInt( f )-8;len=len;
		if( !strcmp( readTag( f ),"WAVE" ) ){
			if( !strcmp( readTag( f ),"fmt " ) ){
				int len2=readInt( f );
				int comp=readShort( f );
				if( comp==1 ){
					int chans=readShort( f );
					int hertz=readInt( f );
					int bytespersec=readInt( f );bytespersec=bytespersec;
					int pad=readShort( f );pad=pad;
					int bits=readShort( f );
					int format=bits/8;
					if( len2>16 ) skipBytes( len2-16,f );
					for(;;){
						const char *p=readTag( f );
						if( feof( f ) ) break;
						int size=readInt( f );
						if( strcmp( p,"data" ) ){
							skipBytes( size,f );
							continue;
						}
						unsigned char *data=(unsigned char*)malloc( size );
						if( fread( data,size,1,f )==1 ){
							*plength=size/(chans*format);
							*pchannels=chans;
							*pformat=format;
							*phertz=hertz;
							return data;
						}
						free( data );
					}
				}
			}
		}
	}
	return 0;
}



//***** oggloader.h *****
unsigned char *LoadOGG( FILE *f,int *length,int *channels,int *format,int *hertz );

//***** oggloader.cpp *****
unsigned char *LoadOGG( FILE *f,int *length,int *channels,int *format,int *hertz ){

	int error;
	stb_vorbis *v=stb_vorbis_open_file( f,0,&error,0 );
	if( !v ) return 0;
	
	stb_vorbis_info info=stb_vorbis_get_info( v );
	
	int limit=info.channels*4096;
	int offset=0,total=limit;

	short *data=(short*)malloc( total*2 );
	
	for(;;){
		int n=stb_vorbis_get_frame_short_interleaved( v,info.channels,data+offset,total-offset );
		if( !n ) break;
	
		offset+=n*info.channels;
		
		if( offset+limit>total ){
			total*=2;
			data=(short*)realloc( data,total*2 );
		}
	}
	
	data=(short*)realloc( data,offset*2 );
	
	*length=offset/info.channels;
	*channels=info.channels;
	*format=2;
	*hertz=info.sample_rate;
	
	stb_vorbis_close(v);
	
	return (unsigned char*)data;
}



//***** glfwgame.h *****

struct BBGlfwVideoMode : public Object{
	int Width;
	int Height;
	int RedBits;
	int GreenBits;
	int BlueBits;
	BBGlfwVideoMode( int w,int h,int r,int g,int b ):Width(w),Height(h),RedBits(r),GreenBits(g),BlueBits(b){}
};

class BBGlfwGame : public BBGame{
public:
	BBGlfwGame();

	static BBGlfwGame *GlfwGame(){ return _glfwGame; }
	
	virtual void SetUpdateRate( int hertz );
	virtual int Millisecs();
	virtual bool PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons );
	virtual void OpenUrl( String url );
	virtual void SetMouseVisible( bool visible );
	
	virtual int GetDeviceWidth();
	virtual int GetDeviceHeight();
	virtual void SetDeviceWindow( int width,int height,int flags );
	virtual Array<BBDisplayMode*> GetDisplayModes();
	virtual BBDisplayMode *GetDesktopMode();
	virtual void SetSwapInterval( int interval );

	virtual String PathToFilePath( String path );
	virtual unsigned char *LoadImageData( String path,int *width,int *height,int *depth );
	virtual unsigned char *LoadAudioData( String path,int *length,int *channels,int *format,int *hertz );
	
	virtual void SetGlfwWindow( int width,int height,int red,int green,int blue,int alpha,int depth,int stencil,bool fullscreen );
	virtual BBGlfwVideoMode *GetGlfwDesktopMode();
	virtual Array<BBGlfwVideoMode*> GetGlfwVideoModes();
	
	virtual void Run();
	
private:
	static BBGlfwGame *_glfwGame;

	double _updatePeriod;
	double _nextUpdate;
	
	int _swapInterval;
	
	void UpdateEvents();
		
protected:
	static int TransKey( int key );
	static int KeyToChar( int key );
	
	static void GLFWCALL OnKey( int key,int action );
	static void GLFWCALL OnChar( int chr,int action );
	static void GLFWCALL OnMouseButton( int button,int action );
	static void GLFWCALL OnMousePos( int x,int y );
	static int  GLFWCALL OnWindowClose();
};

//***** glfwgame.cpp *****

#define _QUOTE(X) #X
#define _STRINGIZE( X ) _QUOTE(X)

enum{
	VKEY_BACKSPACE=8,VKEY_TAB,
	VKEY_ENTER=13,
	VKEY_SHIFT=16,
	VKEY_CONTROL=17,
	VKEY_ESC=27,
	VKEY_SPACE=32,
	VKEY_PAGEUP=33,VKEY_PAGEDOWN,VKEY_END,VKEY_HOME,
	VKEY_LEFT=37,VKEY_UP,VKEY_RIGHT,VKEY_DOWN,
	VKEY_INSERT=45,VKEY_DELETE,
	VKEY_0=48,VKEY_1,VKEY_2,VKEY_3,VKEY_4,VKEY_5,VKEY_6,VKEY_7,VKEY_8,VKEY_9,
	VKEY_A=65,VKEY_B,VKEY_C,VKEY_D,VKEY_E,VKEY_F,VKEY_G,VKEY_H,VKEY_I,VKEY_J,
	VKEY_K,VKEY_L,VKEY_M,VKEY_N,VKEY_O,VKEY_P,VKEY_Q,VKEY_R,VKEY_S,VKEY_T,
	VKEY_U,VKEY_V,VKEY_W,VKEY_X,VKEY_Y,VKEY_Z,
	
	VKEY_LSYS=91,VKEY_RSYS,
	
	VKEY_NUM0=96,VKEY_NUM1,VKEY_NUM2,VKEY_NUM3,VKEY_NUM4,
	VKEY_NUM5,VKEY_NUM6,VKEY_NUM7,VKEY_NUM8,VKEY_NUM9,
	VKEY_NUMMULTIPLY=106,VKEY_NUMADD,VKEY_NUMSLASH,
	VKEY_NUMSUBTRACT,VKEY_NUMDECIMAL,VKEY_NUMDIVIDE,

	VKEY_F1=112,VKEY_F2,VKEY_F3,VKEY_F4,VKEY_F5,VKEY_F6,
	VKEY_F7,VKEY_F8,VKEY_F9,VKEY_F10,VKEY_F11,VKEY_F12,

	VKEY_LSHIFT=160,VKEY_RSHIFT,
	VKEY_LCONTROL=162,VKEY_RCONTROL,
	VKEY_LALT=164,VKEY_RALT,

	VKEY_TILDE=192,VKEY_MINUS=189,VKEY_EQUALS=187,
	VKEY_OPENBRACKET=219,VKEY_BACKSLASH=220,VKEY_CLOSEBRACKET=221,
	VKEY_SEMICOLON=186,VKEY_QUOTES=222,
	VKEY_COMMA=188,VKEY_PERIOD=190,VKEY_SLASH=191
};

BBGlfwGame *BBGlfwGame::_glfwGame;

BBGlfwGame::BBGlfwGame():_updatePeriod(0),_nextUpdate(0),_swapInterval( CFG_GLFW_SWAP_INTERVAL ){
	_glfwGame=this;
}

//***** BBGame *****

void Init_GL_Exts();

int glfwGraphicsSeq=0;

void BBGlfwGame::SetUpdateRate( int updateRate ){
	BBGame::SetUpdateRate( updateRate );
	if( _updateRate ) _updatePeriod=1.0/_updateRate;
	_nextUpdate=0;
}

int BBGlfwGame::Millisecs(){
	return int( glfwGetTime()*1000.0 );
}

bool BBGlfwGame::PollJoystick( int port,Array<Float> joyx,Array<Float> joyy,Array<Float> joyz,Array<bool> buttons ){

	int joy=GLFW_JOYSTICK_1+port;
	if( !glfwGetJoystickParam( joy,GLFW_PRESENT ) ) return false;

	//read axes
	float axes[6];
	memset( axes,0,sizeof(axes) );
	int n_axes=glfwGetJoystickPos( joy,axes,6 );
	joyx[0]=axes[0];joyy[0]=axes[1];joyz[0]=axes[2];
	joyx[1]=axes[3];joyy[1]=axes[4];joyz[1]=axes[5];
	
	//read buttons
	unsigned char buts[32];
	memset( buts,0,sizeof(buts) );
	int n_buts=glfwGetJoystickButtons( joy,buts,32 );
	if( n_buts>12 ){
		for( int i=0;i<8;++i ) buttons[i]=(buts[i]==GLFW_PRESS);
		for( int i=0;i<4;++i ) buttons[i+8]=(buts[n_buts-4+i]==GLFW_PRESS);
		for( int i=0;i<n_buts-12;++i ) buttons[i+12]=(buts[i+8]==GLFW_PRESS);
	}else{
		for( int i=0;i<n_buts;++i ) buttons[i]=(buts[i]=-GLFW_PRESS);
	}
	
	//kludges for device type!
	if( n_axes==5 && n_buts==14 ){
		//XBOX_360?
		joyx[1]=axes[4];
		joyy[1]=-axes[3];
	}else if( n_axes==4 && n_buts==18 ){
		//My Saitek?
		joyy[1]=-joyz[0];
	}
	
	//enough!
	return true;
}

void BBGlfwGame::OpenUrl( String url ){
#if _WIN32
	ShellExecute( HWND_DESKTOP,"open",url.ToCString<char>(),0,0,SW_SHOWNORMAL );
#elif __APPLE__
	if( CFURLRef cfurl=CFURLCreateWithBytes( 0,url.ToCString<UInt8>(),url.Length(),kCFStringEncodingASCII,0 ) ){
		LSOpenCFURLRef( cfurl,0 );
		CFRelease( cfurl );
	}
#elif __linux
	system( ( String( "xdg-open \"" )+url+"\"" ).ToCString<char>() );
#endif
}

void BBGlfwGame::SetMouseVisible( bool visible ){
	if( visible ){
		glfwEnable( GLFW_MOUSE_CURSOR );
	}else{
		glfwDisable( GLFW_MOUSE_CURSOR );
	}
}

String BBGlfwGame::PathToFilePath( String path ){
	if( !path.StartsWith( "monkey:" ) ){
		return path;
	}else if( path.StartsWith( "monkey://data/" ) ){
		return String("./data/")+path.Slice(14);
	}else if( path.StartsWith( "monkey://internal/" ) ){
		return String("./internal/")+path.Slice(18);
	}else if( path.StartsWith( "monkey://external/" ) ){
		return String("./external/")+path.Slice(18);
	}
	return "";
}

unsigned char *BBGlfwGame::LoadImageData( String path,int *width,int *height,int *depth ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;
	
	unsigned char *data=stbi_load_from_file( f,width,height,depth,0 );
	fclose( f );
	
	if( data ) gc_ext_malloced( (*width)*(*height)*(*depth) );
	
	return data;
}

unsigned char *BBGlfwGame::LoadAudioData( String path,int *length,int *channels,int *format,int *hertz ){

	FILE *f=OpenFile( path,"rb" );
	if( !f ) return 0;
	
	unsigned char *data=0;
	
	if( path.ToLower().EndsWith( ".wav" ) ){
		data=LoadWAV( f,length,channels,format,hertz );
	}else if( path.ToLower().EndsWith( ".ogg" ) ){
		data=LoadOGG( f,length,channels,format,hertz );
	}
	fclose( f );
	
	if( data ) gc_ext_malloced( (*length)*(*channels)*(*format) );
	
	return data;
}

//glfw key to monkey key!
int BBGlfwGame::TransKey( int key ){

	if( key>='0' && key<='9' ) return key;
	if( key>='A' && key<='Z' ) return key;

	switch( key ){

	case ' ':return VKEY_SPACE;
	case ';':return VKEY_SEMICOLON;
	case '=':return VKEY_EQUALS;
	case ',':return VKEY_COMMA;
	case '-':return VKEY_MINUS;
	case '.':return VKEY_PERIOD;
	case '/':return VKEY_SLASH;
	case '~':return VKEY_TILDE;
	case '[':return VKEY_OPENBRACKET;
	case ']':return VKEY_CLOSEBRACKET;
	case '\"':return VKEY_QUOTES;
	case '\\':return VKEY_BACKSLASH;
	
	case '`':return VKEY_TILDE;
	case '\'':return VKEY_QUOTES;

	case GLFW_KEY_LSHIFT:
	case GLFW_KEY_RSHIFT:return VKEY_SHIFT;
	case GLFW_KEY_LCTRL:
	case GLFW_KEY_RCTRL:return VKEY_CONTROL;
	
//	case GLFW_KEY_LSHIFT:return VKEY_LSHIFT;
//	case GLFW_KEY_RSHIFT:return VKEY_RSHIFT;
//	case GLFW_KEY_LCTRL:return VKEY_LCONTROL;
//	case GLFW_KEY_RCTRL:return VKEY_RCONTROL;
	
	case GLFW_KEY_BACKSPACE:return VKEY_BACKSPACE;
	case GLFW_KEY_TAB:return VKEY_TAB;
	case GLFW_KEY_ENTER:return VKEY_ENTER;
	case GLFW_KEY_ESC:return VKEY_ESC;
	case GLFW_KEY_INSERT:return VKEY_INSERT;
	case GLFW_KEY_DEL:return VKEY_DELETE;
	case GLFW_KEY_PAGEUP:return VKEY_PAGEUP;
	case GLFW_KEY_PAGEDOWN:return VKEY_PAGEDOWN;
	case GLFW_KEY_HOME:return VKEY_HOME;
	case GLFW_KEY_END:return VKEY_END;
	case GLFW_KEY_UP:return VKEY_UP;
	case GLFW_KEY_DOWN:return VKEY_DOWN;
	case GLFW_KEY_LEFT:return VKEY_LEFT;
	case GLFW_KEY_RIGHT:return VKEY_RIGHT;
	
	case GLFW_KEY_KP_0:return VKEY_NUM0;
	case GLFW_KEY_KP_1:return VKEY_NUM1;
	case GLFW_KEY_KP_2:return VKEY_NUM2;
	case GLFW_KEY_KP_3:return VKEY_NUM3;
	case GLFW_KEY_KP_4:return VKEY_NUM4;
	case GLFW_KEY_KP_5:return VKEY_NUM5;
	case GLFW_KEY_KP_6:return VKEY_NUM6;
	case GLFW_KEY_KP_7:return VKEY_NUM7;
	case GLFW_KEY_KP_8:return VKEY_NUM8;
	case GLFW_KEY_KP_9:return VKEY_NUM9;
	case GLFW_KEY_KP_DIVIDE:return VKEY_NUMDIVIDE;
	case GLFW_KEY_KP_MULTIPLY:return VKEY_NUMMULTIPLY;
	case GLFW_KEY_KP_SUBTRACT:return VKEY_NUMSUBTRACT;
	case GLFW_KEY_KP_ADD:return VKEY_NUMADD;
	case GLFW_KEY_KP_DECIMAL:return VKEY_NUMDECIMAL;
    	
	case GLFW_KEY_F1:return VKEY_F1;
	case GLFW_KEY_F2:return VKEY_F2;
	case GLFW_KEY_F3:return VKEY_F3;
	case GLFW_KEY_F4:return VKEY_F4;
	case GLFW_KEY_F5:return VKEY_F5;
	case GLFW_KEY_F6:return VKEY_F6;
	case GLFW_KEY_F7:return VKEY_F7;
	case GLFW_KEY_F8:return VKEY_F8;
	case GLFW_KEY_F9:return VKEY_F9;
	case GLFW_KEY_F10:return VKEY_F10;
	case GLFW_KEY_F11:return VKEY_F11;
	case GLFW_KEY_F12:return VKEY_F12;
	}
	return 0;
}

//monkey key to special monkey char
int BBGlfwGame::KeyToChar( int key ){
	switch( key ){
	case VKEY_BACKSPACE:
	case VKEY_TAB:
	case VKEY_ENTER:
	case VKEY_ESC:
		return key;
	case VKEY_PAGEUP:
	case VKEY_PAGEDOWN:
	case VKEY_END:
	case VKEY_HOME:
	case VKEY_LEFT:
	case VKEY_UP:
	case VKEY_RIGHT:
	case VKEY_DOWN:
	case VKEY_INSERT:
		return key | 0x10000;
	case VKEY_DELETE:
		return 127;
	}
	return 0;
}

void BBGlfwGame::OnMouseButton( int button,int action ){
	switch( button ){
	case GLFW_MOUSE_BUTTON_LEFT:button=0;break;
	case GLFW_MOUSE_BUTTON_RIGHT:button=1;break;
	case GLFW_MOUSE_BUTTON_MIDDLE:button=2;break;
	default:return;
	}
	int x,y;
	glfwGetMousePos( &x,&y );
	switch( action ){
	case GLFW_PRESS:
		_glfwGame->MouseEvent( BBGameEvent::MouseDown,button,x,y );
		break;
	case GLFW_RELEASE:
		_glfwGame->MouseEvent( BBGameEvent::MouseUp,button,x,y );
		break;
	}
}

void BBGlfwGame::OnMousePos( int x,int y ){
	_game->MouseEvent( BBGameEvent::MouseMove,-1,x,y );
}

int BBGlfwGame::OnWindowClose(){
	_game->KeyEvent( BBGameEvent::KeyDown,0x1b0 );
	_game->KeyEvent( BBGameEvent::KeyUp,0x1b0 );
	return GL_FALSE;
}

void BBGlfwGame::OnKey( int key,int action ){

	key=TransKey( key );
	if( !key ) return;
	
	switch( action ){
	case GLFW_PRESS:
		_glfwGame->KeyEvent( BBGameEvent::KeyDown,key );
		if( int chr=KeyToChar( key ) ) _game->KeyEvent( BBGameEvent::KeyChar,chr );
		break;
	case GLFW_RELEASE:
		_glfwGame->KeyEvent( BBGameEvent::KeyUp,key );
		break;
	}
}

void BBGlfwGame::OnChar( int chr,int action ){

	switch( action ){
	case GLFW_PRESS:
		_glfwGame->KeyEvent( BBGameEvent::KeyChar,chr );
		break;
	}
}

void BBGlfwGame::SetGlfwWindow( int width,int height,int red,int green,int blue,int alpha,int depth,int stencil,bool fullscreen ){

	for( int i=0;i<=GLFW_KEY_LAST;++i ){
		int key=TransKey( i );
		if( key && glfwGetKey( i )==GLFW_PRESS ) KeyEvent( BBGameEvent::KeyUp,key );
	}

	GLFWvidmode desktopMode;
	glfwGetDesktopMode( &desktopMode );

	glfwCloseWindow();
	
	glfwOpenWindowHint( GLFW_REFRESH_RATE,60 );
	glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE,CFG_GLFW_WINDOW_RESIZABLE ? GL_FALSE : GL_TRUE );

	glfwOpenWindow( width,height,red,green,blue,alpha,depth,stencil,fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW );

	++glfwGraphicsSeq;

	if( !fullscreen ){	
		glfwSetWindowPos( (desktopMode.Width-width)/2,(desktopMode.Height-height)/2 );
		glfwSetWindowTitle( _STRINGIZE(CFG_GLFW_WINDOW_TITLE) );
	}

#if CFG_OPENGL_INIT_EXTENSIONS
	Init_GL_Exts();
#endif

	if( _swapInterval>=0 ) glfwSwapInterval( _swapInterval );

	glfwEnable( GLFW_KEY_REPEAT );
	glfwDisable( GLFW_AUTO_POLL_EVENTS );
	glfwSetKeyCallback( OnKey );
	glfwSetCharCallback( OnChar );
	glfwSetMouseButtonCallback( OnMouseButton );
	glfwSetMousePosCallback( OnMousePos );
	glfwSetWindowCloseCallback(	OnWindowClose );
}

Array<BBGlfwVideoMode*> BBGlfwGame::GetGlfwVideoModes(){
	GLFWvidmode modes[1024];
	int n=glfwGetVideoModes( modes,1024 );
	Array<BBGlfwVideoMode*> bbmodes( n );
	for( int i=0;i<n;++i ){
		bbmodes[i]=new BBGlfwVideoMode( modes[i].Width,modes[i].Height,modes[i].RedBits,modes[i].GreenBits,modes[i].BlueBits );
	}
	return bbmodes;
}

BBGlfwVideoMode *BBGlfwGame::GetGlfwDesktopMode(){
	GLFWvidmode mode;
	glfwGetDesktopMode( &mode );
	return new BBGlfwVideoMode( mode.Width,mode.Height,mode.RedBits,mode.GreenBits,mode.BlueBits );
}

int BBGlfwGame::GetDeviceWidth(){
	int width,height;
	glfwGetWindowSize( &width,&height );
	return width;
}

int BBGlfwGame::GetDeviceHeight(){
	int width,height;
	glfwGetWindowSize( &width,&height );
	return height;
}

void BBGlfwGame::SetDeviceWindow( int width,int height,int flags ){

	SetGlfwWindow( width,height,8,8,8,0,CFG_OPENGL_DEPTH_BUFFER_ENABLED ? 32 : 0,0,(flags&1)!=0 );
}

Array<BBDisplayMode*> BBGlfwGame::GetDisplayModes(){

	GLFWvidmode vmodes[1024];
	int n=glfwGetVideoModes( vmodes,1024 );
	Array<BBDisplayMode*> modes( n );
	for( int i=0;i<n;++i ) modes[i]=new BBDisplayMode( vmodes[i].Width,vmodes[i].Height );
	return modes;
}

BBDisplayMode *BBGlfwGame::GetDesktopMode(){

	GLFWvidmode vmode;
	glfwGetDesktopMode( &vmode );
	return new BBDisplayMode( vmode.Width,vmode.Height );
}

void BBGlfwGame::SetSwapInterval( int interval ){
	_swapInterval=interval;
	if( _swapInterval>=0 ) glfwSwapInterval( _swapInterval );
}

void BBGlfwGame::UpdateEvents(){
	if( _suspended ){
		glfwWaitEvents();
	}else{
		glfwPollEvents();
	}
	if( glfwGetWindowParam( GLFW_ACTIVE ) ){
		if( _suspended ){
			ResumeGame();
			_nextUpdate=0;
		}
	}else if( glfwGetWindowParam( GLFW_ICONIFIED ) || CFG_MOJO_AUTO_SUSPEND_ENABLED ){
		if( !_suspended ){
			SuspendGame();
			_nextUpdate=0;
		}
	}
}

void BBGlfwGame::Run(){

#if	CFG_GLFW_WINDOW_WIDTH && CFG_GLFW_WINDOW_HEIGHT

	SetGlfwWindow( CFG_GLFW_WINDOW_WIDTH,CFG_GLFW_WINDOW_HEIGHT,8,8,8,0,CFG_OPENGL_DEPTH_BUFFER_ENABLED ? 32 : 0,0,CFG_GLFW_WINDOW_FULLSCREEN );

#endif

	StartGame();
	
	while( glfwGetWindowParam( GLFW_OPENED ) ){
	
		RenderGame();
		glfwSwapBuffers();
		
		if( _nextUpdate ){
			double delay=_nextUpdate-glfwGetTime();
			if( delay>0 ) glfwSleep( delay );
		}
		
		//Update user events
		UpdateEvents();

		//App suspended?		
		if( _suspended ) continue;

		//'Go nuts' mode!
		if( !_updateRate ){
			UpdateGame();
			continue;
		}
		
		//Reset update timer?
		if( !_nextUpdate ) _nextUpdate=glfwGetTime();
		
		//Catch up updates...
		int i=0;
		for( ;i<4;++i ){
		
			UpdateGame();
			if( !_nextUpdate ) break;
			
			_nextUpdate+=_updatePeriod;
			
			if( _nextUpdate>glfwGetTime() ) break;
		}
		
		if( i==4 ) _nextUpdate=0;
	}
}



//***** monkeygame.h *****

class BBMonkeyGame : public BBGlfwGame{
public:

	static void Main( int args,const char *argv[] );
};

//***** monkeygame.cpp *****

#define _QUOTE(X) #X
#define _STRINGIZE(X) _QUOTE(X)

void BBMonkeyGame::Main( int argc,const char *argv[] ){

	if( !glfwInit() ){
		puts( "glfwInit failed" );
		exit(-1);
	}

	BBMonkeyGame *game=new BBMonkeyGame();
	
	try{
	
		bb_std_main( argc,argv );
		
	}catch( ThrowableObject *ex ){
	
		glfwTerminate();
		
		game->Die( ex );
		
		return;
	}

	if( game->Delegate() ) game->Run();
	
	glfwTerminate();
}


// GLFW mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.

//***** gxtkGraphics.h *****

class gxtkSurface;

class gxtkGraphics : public Object{
public:

	enum{
		MAX_VERTS=1024,
		MAX_QUADS=(MAX_VERTS/4)
	};

	int width;
	int height;

	int colorARGB;
	float r,g,b,alpha;
	float ix,iy,jx,jy,tx,ty;
	bool tformed;

	float vertices[MAX_VERTS*5];
	unsigned short quadIndices[MAX_QUADS*6];

	int primType;
	int vertCount;
	gxtkSurface *primSurf;
	
	gxtkGraphics();
	
	void Flush();
	float *Begin( int type,int count,gxtkSurface *surf );
	
	//***** GXTK API *****
	virtual int Width();
	virtual int Height();
	
	virtual int  BeginRender();
	virtual void EndRender();
	virtual void DiscardGraphics();

	virtual gxtkSurface *LoadSurface( String path );
	virtual gxtkSurface *LoadSurface__UNSAFE__( gxtkSurface *surface,String path );
	virtual gxtkSurface *CreateSurface( int width,int height );
	
	virtual int Cls( float r,float g,float b );
	virtual int SetAlpha( float alpha );
	virtual int SetColor( float r,float g,float b );
	virtual int SetBlend( int blend );
	virtual int SetScissor( int x,int y,int w,int h );
	virtual int SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty );
	
	virtual int DrawPoint( float x,float y );
	virtual int DrawRect( float x,float y,float w,float h );
	virtual int DrawLine( float x1,float y1,float x2,float y2 );
	virtual int DrawOval( float x1,float y1,float x2,float y2 );
	virtual int DrawPoly( Array<Float> verts );
	virtual int DrawPoly2( Array<Float> verts,gxtkSurface *surface,int srcx,int srcy );
	virtual int DrawSurface( gxtkSurface *surface,float x,float y );
	virtual int DrawSurface2( gxtkSurface *surface,float x,float y,int srcx,int srcy,int srcw,int srch );
	
	virtual int ReadPixels( Array<int> pixels,int x,int y,int width,int height,int offset,int pitch );
	virtual int WritePixels2( gxtkSurface *surface,Array<int> pixels,int x,int y,int width,int height,int offset,int pitch );
};

class gxtkSurface : public Object{
public:
	unsigned char *data;
	int width;
	int height;
	int depth;
	int format;
	int seq;
	
	GLuint texture;
	float uscale;
	float vscale;
	
	gxtkSurface();
	
	void SetData( unsigned char *data,int width,int height,int depth );
	void SetSubData( int x,int y,int w,int h,unsigned *src,int pitch );
	void Bind();
	
	~gxtkSurface();
	
	//***** GXTK API *****
	virtual int Discard();
	virtual int Width();
	virtual int Height();
	virtual int Loaded();
	virtual bool OnUnsafeLoadComplete();
};

//***** gxtkGraphics.cpp *****

#ifndef GL_BGRA
#define GL_BGRA  0x80e1
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812f
#endif

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

static int Pow2Size( int n ){
	int i=1;
	while( i<n ) i+=i;
	return i;
}

gxtkGraphics::gxtkGraphics(){

	width=height=0;
#ifdef _glfw3_h_
	GLFWwindow *window=BBGlfwGame::GlfwGame()->GetGLFWwindow();
	if( window ) glfwGetWindowSize( BBGlfwGame::GlfwGame()->GetGLFWwindow(),&width,&height );
#else
	glfwGetWindowSize( &width,&height );
#endif
	
	if( CFG_OPENGL_GLES20_ENABLED ) return;
	
	for( int i=0;i<MAX_QUADS;++i ){
		quadIndices[i*6  ]=(short)(i*4);
		quadIndices[i*6+1]=(short)(i*4+1);
		quadIndices[i*6+2]=(short)(i*4+2);
		quadIndices[i*6+3]=(short)(i*4);
		quadIndices[i*6+4]=(short)(i*4+2);
		quadIndices[i*6+5]=(short)(i*4+3);
	}
}

void gxtkGraphics::Flush(){
	if( !vertCount ) return;

	if( primSurf ){
		glEnable( GL_TEXTURE_2D );
		primSurf->Bind();
	}
		
	switch( primType ){
	case 1:
		glDrawArrays( GL_POINTS,0,vertCount );
		break;
	case 2:
		glDrawArrays( GL_LINES,0,vertCount );
		break;
	case 3:
		glDrawArrays( GL_TRIANGLES,0,vertCount );
		break;
	case 4:
		glDrawElements( GL_TRIANGLES,vertCount/4*6,GL_UNSIGNED_SHORT,quadIndices );
		break;
	default:
		for( int j=0;j<vertCount;j+=primType ){
			glDrawArrays( GL_TRIANGLE_FAN,j,primType );
		}
		break;
	}

	if( primSurf ){
		glDisable( GL_TEXTURE_2D );
	}

	vertCount=0;
}

float *gxtkGraphics::Begin( int type,int count,gxtkSurface *surf ){
	if( primType!=type || primSurf!=surf || vertCount+count>MAX_VERTS ){
		Flush();
		primType=type;
		primSurf=surf;
	}
	float *vp=vertices+vertCount*5;
	vertCount+=count;
	return vp;
}

//***** GXTK API *****

int gxtkGraphics::Width(){
	return width;
}

int gxtkGraphics::Height(){
	return height;
}

int gxtkGraphics::BeginRender(){

	width=height=0;
#ifdef _glfw3_h_
	glfwGetWindowSize( BBGlfwGame::GlfwGame()->GetGLFWwindow(),&width,&height );
#else
	glfwGetWindowSize( &width,&height );
#endif
	
	if( CFG_OPENGL_GLES20_ENABLED ) return 0;
	
	glViewport( 0,0,width,height );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0,width,height,0,-1,1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2,GL_FLOAT,20,&vertices[0] );	
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2,GL_FLOAT,20,&vertices[2] );
	
	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4,GL_UNSIGNED_BYTE,20,&vertices[4] );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE,GL_ONE_MINUS_SRC_ALPHA );
	
	glDisable( GL_TEXTURE_2D );
	
	vertCount=0;
	
	return 1;
}

void gxtkGraphics::EndRender(){
	if( !CFG_OPENGL_GLES20_ENABLED ) Flush();
}

void gxtkGraphics::DiscardGraphics(){
}

int gxtkGraphics::Cls( float r,float g,float b ){
	vertCount=0;

	glClearColor( r/255.0f,g/255.0f,b/255.0f,1 );
	glClear( GL_COLOR_BUFFER_BIT );

	return 0;
}

int gxtkGraphics::SetAlpha( float alpha ){
	this->alpha=alpha;
	
	int a=int(alpha*255);
	
	colorARGB=(a<<24) | (int(b*alpha)<<16) | (int(g*alpha)<<8) | int(r*alpha);
	
	return 0;
}

int gxtkGraphics::SetColor( float r,float g,float b ){
	this->r=r;
	this->g=g;
	this->b=b;

	int a=int(alpha*255);
	
	colorARGB=(a<<24) | (int(b*alpha)<<16) | (int(g*alpha)<<8) | int(r*alpha);
	
	return 0;
}

int gxtkGraphics::SetBlend( int blend ){

	Flush();
	
	switch( blend ){
	case 1:
		glBlendFunc( GL_ONE,GL_ONE );
		break;
	default:
		glBlendFunc( GL_ONE,GL_ONE_MINUS_SRC_ALPHA );
	}

	return 0;
}

int gxtkGraphics::SetScissor( int x,int y,int w,int h ){

	Flush();
	
	if( x!=0 || y!=0 || w!=Width() || h!=Height() ){
		glEnable( GL_SCISSOR_TEST );
		y=Height()-y-h;
		glScissor( x,y,w,h );
	}else{
		glDisable( GL_SCISSOR_TEST );
	}
	return 0;
}

int gxtkGraphics::SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty ){

	tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);

	this->ix=ix;this->iy=iy;this->jx=jx;this->jy=jy;this->tx=tx;this->ty=ty;

	return 0;
}

int gxtkGraphics::DrawPoint( float x,float y ){

	if( tformed ){
		float px=x;
		x=px * ix + y * jx + tx;
		y=px * iy + y * jy + ty;
	}
	
	float *vp=Begin( 1,1,0 );
	
	vp[0]=x;vp[1]=y;(int&)vp[4]=colorARGB;

	return 0;	
}
	
int gxtkGraphics::DrawLine( float x0,float y0,float x1,float y1 ){

	if( tformed ){
		float tx0=x0,tx1=x1;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
	}
	
	float *vp=Begin( 2,2,0 );

	vp[0]=x0;vp[1]=y0;(int&)vp[4]=colorARGB;
	vp[5]=x1;vp[6]=y1;(int&)vp[9]=colorARGB;
	
	return 0;
}

int gxtkGraphics::DrawRect( float x,float y,float w,float h ){

	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=Begin( 4,4,0 );

	vp[0 ]=x0;vp[1 ]=y0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;(int&)vp[19]=colorARGB;

	return 0;
}

int gxtkGraphics::DrawOval( float x,float y,float w,float h ){
	
	float xr=w/2.0f;
	float yr=h/2.0f;

	int n;
	if( tformed ){
		float dx_x=xr * ix;
		float dx_y=xr * iy;
		float dx=sqrtf( dx_x*dx_x+dx_y*dx_y );
		float dy_x=yr * jx;
		float dy_y=yr * jy;
		float dy=sqrtf( dy_x*dy_x+dy_y*dy_y );
		n=(int)( dx+dy );
	}else{
		n=(int)( abs( xr )+abs( yr ) );
	}
	
	if( n<12 ){
		n=12;
	}else if( n>MAX_VERTS ){
		n=MAX_VERTS;
	}else{
		n&=~3;
	}

	float x0=x+xr,y0=y+yr;
	
	float *vp=Begin( n,n,0 );

	for( int i=0;i<n;++i ){
	
		float th=i * 6.28318531f / n;

		float px=x0+cosf( th ) * xr;
		float py=y0-sinf( th ) * yr;
		
		if( tformed ){
			float ppx=px;
			px=ppx * ix + py * jx + tx;
			py=ppx * iy + py * jy + ty;
		}
		
		vp[0]=px;vp[1]=py;(int&)vp[4]=colorARGB;
		vp+=5;
	}
	
	return 0;
}

int gxtkGraphics::DrawPoly( Array<Float> verts ){

	int n=verts.Length()/2;
	if( n<1 || n>MAX_VERTS ) return 0;
	
	float *vp=Begin( n,n,0 );
	
	for( int i=0;i<n;++i ){
		int j=i*2;
		if( tformed ){
			vp[0]=verts[j] * ix + verts[j+1] * jx + tx;
			vp[1]=verts[j] * iy + verts[j+1] * jy + ty;
		}else{
			vp[0]=verts[j];
			vp[1]=verts[j+1];
		}
		(int&)vp[4]=colorARGB;
		vp+=5;
	}

	return 0;
}

int gxtkGraphics::DrawPoly2( Array<Float> verts,gxtkSurface *surface,int srcx,int srcy ){

	int n=verts.Length()/4;
	if( n<1 || n>MAX_VERTS ) return 0;
		
	float *vp=Begin( n,n,surface );
	
	for( int i=0;i<n;++i ){
		int j=i*4;
		if( tformed ){
			vp[0]=verts[j] * ix + verts[j+1] * jx + tx;
			vp[1]=verts[j] * iy + verts[j+1] * jy + ty;
		}else{
			vp[0]=verts[j];
			vp[1]=verts[j+1];
		}
		vp[2]=(srcx+verts[j+2])*surface->uscale;
		vp[3]=(srcy+verts[j+3])*surface->vscale;
		(int&)vp[4]=colorARGB;
		vp+=5;
	}
	
	return 0;
}

int gxtkGraphics::DrawSurface( gxtkSurface *surf,float x,float y ){
	
	float w=surf->Width();
	float h=surf->Height();
	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;
	float u0=0,u1=w*surf->uscale;
	float v0=0,v1=h*surf->vscale;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=Begin( 4,4,surf );
	
	vp[0 ]=x0;vp[1 ]=y0;vp[2 ]=u0;vp[3 ]=v0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;vp[7 ]=u1;vp[8 ]=v0;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;vp[12]=u1;vp[13]=v1;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;vp[17]=u0;vp[18]=v1;(int&)vp[19]=colorARGB;
	
	return 0;
}

int gxtkGraphics::DrawSurface2( gxtkSurface *surf,float x,float y,int srcx,int srcy,int srcw,int srch ){
	
	float w=srcw;
	float h=srch;
	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;
	float u0=srcx*surf->uscale,u1=(srcx+srcw)*surf->uscale;
	float v0=srcy*surf->vscale,v1=(srcy+srch)*surf->vscale;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=Begin( 4,4,surf );
	
	vp[0 ]=x0;vp[1 ]=y0;vp[2 ]=u0;vp[3 ]=v0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;vp[7 ]=u1;vp[8 ]=v0;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;vp[12]=u1;vp[13]=v1;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;vp[17]=u0;vp[18]=v1;(int&)vp[19]=colorARGB;
	
	return 0;
}
	
int gxtkGraphics::ReadPixels( Array<int> pixels,int x,int y,int width,int height,int offset,int pitch ){

	Flush();

	unsigned *p=(unsigned*)malloc(width*height*4);

	glReadPixels( x,this->height-y-height,width,height,GL_BGRA,GL_UNSIGNED_BYTE,p );
	
	for( int py=0;py<height;++py ){
		memcpy( &pixels[offset+py*pitch],&p[(height-py-1)*width],width*4 );
	}
	
	free( p );
	
	return 0;
}

int gxtkGraphics::WritePixels2( gxtkSurface *surface,Array<int> pixels,int x,int y,int width,int height,int offset,int pitch ){

	Flush();
	
	surface->SetSubData( x,y,width,height,(unsigned*)&pixels[offset],pitch );
	
	return 0;
}

//***** gxtkSurface *****

gxtkSurface::gxtkSurface():data(0),width(0),height(0),depth(0),format(0),seq(-1),texture(0),uscale(0),vscale(0){
}

gxtkSurface::~gxtkSurface(){
	Discard();
}

int gxtkSurface::Discard(){
	if( seq==glfwGraphicsSeq ){
		glDeleteTextures( 1,&texture );
		seq=-1;
	}
	if( data ){
		free( data );
		data=0;
	}
	return 0;
}

int gxtkSurface::Width(){
	return width;
}

int gxtkSurface::Height(){
	return height;
}

int gxtkSurface::Loaded(){
	return 1;
}

//Careful! Can't call any GL here as it may be executing off-thread.
//
void gxtkSurface::SetData( unsigned char *data,int width,int height,int depth ){

	this->data=data;
	this->width=width;
	this->height=height;
	this->depth=depth;
	
	unsigned char *p=data;
	int n=width*height;
	
	switch( depth ){
	case 1:
		format=GL_LUMINANCE;
		break;
	case 2:
		format=GL_LUMINANCE_ALPHA;
		if( data ){
			while( n-- ){	//premultiply alpha
				p[0]=p[0]*p[1]/255;
				p+=2;
			}
		}
		break;
	case 3:
		format=GL_RGB;
		break;
	case 4:
		format=GL_RGBA;
		if( data ){
			while( n-- ){	//premultiply alpha
				p[0]=p[0]*p[3]/255;
				p[1]=p[1]*p[3]/255;
				p[2]=p[2]*p[3]/255;
				p+=4;
			}
		}
		break;
	}
}

void gxtkSurface::SetSubData( int x,int y,int w,int h,unsigned *src,int pitch ){
	if( format!=GL_RGBA ) return;
	
	if( !data ) data=(unsigned char*)malloc( width*height*4 );
	
	unsigned *dst=(unsigned*)data+y*width+x;
	
	for( int py=0;py<h;++py ){
		unsigned *d=dst+py*width;
		unsigned *s=src+py*pitch;
		for( int px=0;px<w;++px ){
			unsigned p=*s++;
			unsigned a=p>>24;
			*d++=(a<<24) | ((p>>0&0xff)*a/255<<16) | ((p>>8&0xff)*a/255<<8) | ((p>>16&0xff)*a/255);
		}
	}
	
	if( seq==glfwGraphicsSeq ){
		glBindTexture( GL_TEXTURE_2D,texture );
		glPixelStorei( GL_UNPACK_ALIGNMENT,1 );
		if( width==pitch ){
			glTexSubImage2D( GL_TEXTURE_2D,0,x,y,w,h,format,GL_UNSIGNED_BYTE,dst );
		}else{
			for( int py=0;py<h;++py ){
				glTexSubImage2D( GL_TEXTURE_2D,0,x,y+py,w,1,format,GL_UNSIGNED_BYTE,dst+py*width );
			}
		}
	}
}

void gxtkSurface::Bind(){

	if( !glfwGraphicsSeq ) return;
	
	if( seq==glfwGraphicsSeq ){
		glBindTexture( GL_TEXTURE_2D,texture );
		return;
	}
	
	seq=glfwGraphicsSeq;
	
	glGenTextures( 1,&texture );
	glBindTexture( GL_TEXTURE_2D,texture );
	
	if( CFG_MOJO_IMAGE_FILTERING_ENABLED ){
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	}else{
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
	}

	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE );

	int texwidth=width;
	int texheight=height;
	
	glTexImage2D( GL_TEXTURE_2D,0,format,texwidth,texheight,0,format,GL_UNSIGNED_BYTE,0 );
	if( glGetError()!=GL_NO_ERROR ){
		texwidth=Pow2Size( width );
		texheight=Pow2Size( height );
		glTexImage2D( GL_TEXTURE_2D,0,format,texwidth,texheight,0,format,GL_UNSIGNED_BYTE,0 );
	}
	
	uscale=1.0/texwidth;
	vscale=1.0/texheight;
	
	if( data ){
		glPixelStorei( GL_UNPACK_ALIGNMENT,1 );
		glTexSubImage2D( GL_TEXTURE_2D,0,0,0,width,height,format,GL_UNSIGNED_BYTE,data );
	}
}

bool gxtkSurface::OnUnsafeLoadComplete(){
	Bind();
	return true;
}

gxtkSurface *gxtkGraphics::LoadSurface__UNSAFE__( gxtkSurface *surface,String path ){
	int width,height,depth;
	unsigned char *data=BBGlfwGame::GlfwGame()->LoadImageData( path,&width,&height,&depth );
	if( !data ) return 0;
	surface->SetData( data,width,height,depth );
	return surface;
}

gxtkSurface *gxtkGraphics::LoadSurface( String path ){
	gxtkSurface *surf=LoadSurface__UNSAFE__( new gxtkSurface(),path );
	if( !surf ) return 0;
	surf->Bind();
	return surf;
}

gxtkSurface *gxtkGraphics::CreateSurface( int width,int height ){
	gxtkSurface *surf=new gxtkSurface();
	surf->SetData( 0,width,height,4 );
	surf->Bind();
	return surf;
}

//***** gxtkAudio.h *****

class gxtkSample;

class gxtkChannel{
public:
	ALuint source;
	gxtkSample *sample;
	int flags;
	int state;
	
	int AL_Source();
};

class gxtkAudio : public Object{
public:
	static gxtkAudio *audio;
	
	ALCdevice *alcDevice;
	ALCcontext *alcContext;
	gxtkChannel channels[33];

	gxtkAudio();

	virtual void mark();

	//***** GXTK API *****
	virtual int Suspend();
	virtual int Resume();

	virtual gxtkSample *LoadSample__UNSAFE__( gxtkSample *sample,String path );
	virtual gxtkSample *LoadSample( String path );
	virtual int PlaySample( gxtkSample *sample,int channel,int flags );

	virtual int StopChannel( int channel );
	virtual int PauseChannel( int channel );
	virtual int ResumeChannel( int channel );
	virtual int ChannelState( int channel );
	virtual int SetVolume( int channel,float volume );
	virtual int SetPan( int channel,float pan );
	virtual int SetRate( int channel,float rate );
	
	virtual int PlayMusic( String path,int flags );
	virtual int StopMusic();
	virtual int PauseMusic();
	virtual int ResumeMusic();
	virtual int MusicState();
	virtual int SetMusicVolume( float volume );
};

class gxtkSample : public Object{
public:
	ALuint al_buffer;

	gxtkSample();
	gxtkSample( ALuint buf );
	~gxtkSample();
	
	void SetBuffer( ALuint buf );
	
	//***** GXTK API *****
	virtual int Discard();
};

//***** gxtkAudio.cpp *****

gxtkAudio *gxtkAudio::audio;

static std::vector<ALuint> discarded;

static void FlushDiscarded(){

	if( !discarded.size() ) return;
	
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&gxtkAudio::audio->channels[i];
		if( chan->state ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_STOPPED ) alSourcei( chan->source,AL_BUFFER,0 );
		}
	}
	
	std::vector<ALuint> out;
	
	for( int i=0;i<discarded.size();++i ){
		ALuint buf=discarded[i];
		alDeleteBuffers( 1,&buf );
		ALenum err=alGetError();
		if( err==AL_NO_ERROR ){
//			printf( "alDeleteBuffers OK!\n" );fflush( stdout );
		}else{
//			printf( "alDeleteBuffers failed...\n" );fflush( stdout );
			out.push_back( buf );
		}
	}
	discarded=out;
}

int gxtkChannel::AL_Source(){
	if( source ) return source;

	/*	
	static int n;
	if( ++n<17 ){
		alGetError();
		alGenSources( 1,&source );
		if( alGetError()==AL_NO_ERROR ) return source;
	}
	*/
	
	alGetError();
	alGenSources( 1,&source );
	if( alGetError()==AL_NO_ERROR ) return source;
	
	//couldn't create source...steal a free source...?
	//
	source=0;
	for( int i=0;i<32;++i ){
		gxtkChannel *chan=&gxtkAudio::audio->channels[i];
		if( !chan->source || gxtkAudio::audio->ChannelState( i ) ) continue;
//		puts( "Stealing source!" );
		source=chan->source;
		chan->source=0;
		break;
	}
	return source;
}

gxtkAudio::gxtkAudio(){

	audio=this;

	if( alcDevice=alcOpenDevice( 0 ) ){
		if( alcContext=alcCreateContext( alcDevice,0 ) ){
			if( alcMakeContextCurrent( alcContext ) ){
				//alc all go!
			}else{
				bbPrint( "OpenAl error: alcMakeContextCurrent failed" );
			}
		}else{
			bbPrint( "OpenAl error: alcCreateContext failed" );
		}
	}else{
		bbPrint( "OpenAl error: alcOpenDevice failed" );
	}

	alDistanceModel( AL_NONE );
	
	memset( channels,0,sizeof(channels) );

	channels[32].AL_Source();
}

void gxtkAudio::mark(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state!=0 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state!=AL_STOPPED ) gc_mark( chan->sample );
		}
	}
}

int gxtkAudio::Suspend(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state==1 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_PLAYING ) alSourcePause( chan->source );
		}
	}
	return 0;
}

int gxtkAudio::Resume(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state==1 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_PAUSED ) alSourcePlay( chan->source );
		}
	}
	return 0;
}

gxtkSample *gxtkAudio::LoadSample__UNSAFE__( gxtkSample *sample,String path ){

	int length=0;
	int channels=0;
	int format=0;
	int hertz=0;
	unsigned char *data=BBGlfwGame::GlfwGame()->LoadAudioData( path,&length,&channels,&format,&hertz );
	if( !data ) return 0;
	
	int al_format=0;
	if( format==1 && channels==1 ){
		al_format=AL_FORMAT_MONO8;
	}else if( format==1 && channels==2 ){
		al_format=AL_FORMAT_STEREO8;
	}else if( format==2 && channels==1 ){
		al_format=AL_FORMAT_MONO16;
	}else if( format==2 && channels==2 ){
		al_format=AL_FORMAT_STEREO16;
	}
	
	int size=length*channels*format;
	
	ALuint al_buffer;
	alGenBuffers( 1,&al_buffer );
	alBufferData( al_buffer,al_format,data,size,hertz );
	free( data );
	
	sample->SetBuffer( al_buffer );
	return sample;
}

gxtkSample *gxtkAudio::LoadSample( String path ){

	FlushDiscarded();

	return LoadSample__UNSAFE__( new gxtkSample(),path );
}

int gxtkAudio::PlaySample( gxtkSample *sample,int channel,int flags ){

	FlushDiscarded();
	
	gxtkChannel *chan=&channels[channel];
	
	if( !chan->AL_Source() ) return -1;
	
	alSourceStop( chan->source );
	alSourcei( chan->source,AL_BUFFER,sample->al_buffer );
	alSourcei( chan->source,AL_LOOPING,flags ? 1 : 0 );
	alSourcePlay( chan->source );
	
	gc_assign( chan->sample,sample );

	chan->flags=flags;
	chan->state=1;

	return 0;
}

int gxtkAudio::StopChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state!=0 ){
		alSourceStop( chan->source );
		chan->state=0;
	}
	return 0;
}

int gxtkAudio::PauseChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state==1 ){
		int state=0;
		alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
		if( state==AL_STOPPED ){
			chan->state=0;
		}else{
			alSourcePause( chan->source );
			chan->state=2;
		}
	}
	return 0;
}

int gxtkAudio::ResumeChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state==2 ){
		alSourcePlay( chan->source );
		chan->state=1;
	}
	return 0;
}

int gxtkAudio::ChannelState( int channel ){
	gxtkChannel *chan=&channels[channel];
	
	if( chan->state==1 ){
		int state=0;
		alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
		if( state==AL_STOPPED ) chan->state=0;
	}
	return chan->state;
}

int gxtkAudio::SetVolume( int channel,float volume ){
	gxtkChannel *chan=&channels[channel];

	alSourcef( chan->AL_Source(),AL_GAIN,volume );
	return 0;
}

int gxtkAudio::SetPan( int channel,float pan ){
	gxtkChannel *chan=&channels[channel];
	
	float x=sinf( pan ),y=0,z=-cosf( pan );
	alSource3f( chan->AL_Source(),AL_POSITION,x,y,z );
	return 0;
}

int gxtkAudio::SetRate( int channel,float rate ){
	gxtkChannel *chan=&channels[channel];

	alSourcef( chan->AL_Source(),AL_PITCH,rate );
	return 0;
}

int gxtkAudio::PlayMusic( String path,int flags ){
	StopMusic();
	
	gxtkSample *music=LoadSample( path );
	if( !music ) return -1;
	
	PlaySample( music,32,flags );
	return 0;
}

int gxtkAudio::StopMusic(){
	StopChannel( 32 );
	return 0;
}

int gxtkAudio::PauseMusic(){
	PauseChannel( 32 );
	return 0;
}

int gxtkAudio::ResumeMusic(){
	ResumeChannel( 32 );
	return 0;
}

int gxtkAudio::MusicState(){
	return ChannelState( 32 );
}

int gxtkAudio::SetMusicVolume( float volume ){
	SetVolume( 32,volume );
	return 0;
}

gxtkSample::gxtkSample():
al_buffer(0){
}

gxtkSample::gxtkSample( ALuint buf ):
al_buffer(buf){
}

gxtkSample::~gxtkSample(){
	puts( "Discarding sample" );
	Discard();
}

void gxtkSample::SetBuffer( ALuint buf ){
	al_buffer=buf;
}

int gxtkSample::Discard(){
	if( al_buffer ){
		discarded.push_back( al_buffer );
		al_buffer=0;
	}
	return 0;
}


// ***** thread.h *****

#if __cplusplus_winrt

using namespace Windows::System::Threading;

#endif

class BBThread : public Object{
public:
	Object *result;
	
	BBThread();
	~BBThread();
	
	virtual void Start();
	virtual bool IsRunning();
	virtual Object *Result();
	virtual void SetResult( Object *result );
	
	virtual void Run__UNSAFE__();
	
	virtual void Wait();
	
private:

	enum{
		INIT=0,
		RUNNING=1,
		FINISHED=2
	};

	
	int _state;
	Object *_result;
	
#if __cplusplus_winrt

	friend class Launcher;

	class Launcher{
	
		friend class BBThread;
		BBThread *_thread;
		
		Launcher( BBThread *thread ):_thread(thread){
		}
		
		public:
		void operator()( IAsyncAction ^operation ){
			_thread->Run__UNSAFE__();
			_thread->_state=FINISHED;
		} 
	};

#elif _WIN32

	DWORD _id;
	HANDLE _handle;
	
	static DWORD WINAPI run( void *p );
	
#else

	pthread_t _handle;
	
	static void *run( void *p );
	
#endif

};

// ***** thread.cpp *****

BBThread::BBThread():_result( 0 ),_state( INIT ){
}

BBThread::~BBThread(){
	Wait();
}

bool BBThread::IsRunning(){
	return _state==RUNNING;
}

void BBThread::SetResult( Object *result ){
	_result=result;
}

Object *BBThread::Result(){
	return _result;
}

void BBThread::Run__UNSAFE__(){
}

#if __cplusplus_winrt

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	if( _state==FINISHED ) {}

	_result=0;
	
	_state=RUNNING;
	
	Launcher launcher( this );
	
	auto handler=ref new WorkItemHandler( launcher );
	
	ThreadPool::RunAsync( handler );
}

void BBThread::Wait(){
//	exit( -1 );
}

#elif _WIN32

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	if( _state==FINISHED ) CloseHandle( _handle );

	_state=RUNNING;

	_handle=CreateThread( 0,0,run,this,0,&_id );
	
//	_handle=CreateThread( 0,0,run,this,CREATE_SUSPENDED,&_id );
//	SetThreadPriority( _handle,THREAD_PRIORITY_ABOVE_NORMAL );
//	ResumeThread( _handle );
}

void BBThread::Wait(){
	if( _state==INIT ) return;

	WaitForSingleObject( _handle,INFINITE );
	CloseHandle( _handle );

	_state=INIT;
}

DWORD WINAPI BBThread::run( void *p ){
	BBThread *thread=(BBThread*)p;

	thread->Run__UNSAFE__();
	
	thread->_state=FINISHED;
	return 0;
}

#else

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	if( _state==FINISHED ) pthread_join( _handle,0 );

	_result=0;
		
	_state=RUNNING;
	
	pthread_create( &_handle,0,run,this );
}

void BBThread::Wait(){
	if( _state==INIT ) return;
	
	pthread_join( _handle,0 );
	
	_state=INIT;
}

void *BBThread::run( void *p ){
	BBThread *thread=(BBThread*)p;

	thread->Run__UNSAFE__();

	thread->_state=FINISHED;
	return 0;
}

#endif


// ***** databuffer.h *****

class BBDataBuffer : public Object{
public:
	
	BBDataBuffer();
	
	~BBDataBuffer();
	
	bool _New( int length,void *data=0 );
	
	bool _Load( String path );
	
	void _LoadAsync( String path,BBThread *thread );

	void Discard();
	
	const void *ReadPointer( int offset=0 ){
		return _data+offset;
	}
	
	void *WritePointer( int offset=0 ){
		return _data+offset;
	}
	
	int Length(){
		return _length;
	}
	
	void PokeByte( int addr,int value ){
		*(_data+addr)=value;
	}

	void PokeShort( int addr,int value ){
		*(short*)(_data+addr)=value;
	}
	
	void PokeInt( int addr,int value ){
		*(int*)(_data+addr)=value;
	}
	
	void PokeFloat( int addr,float value ){
		*(float*)(_data+addr)=value;
	}

	int PeekByte( int addr ){
		return *(_data+addr);
	}
	
	int PeekShort( int addr ){
		return *(short*)(_data+addr);
	}
	
	int PeekInt( int addr ){
		return *(int*)(_data+addr);
	}
	
	float PeekFloat( int addr ){
		return *(float*)(_data+addr);
	}
	
private:
	signed char *_data;
	int _length;
};

// ***** databuffer.cpp *****

BBDataBuffer::BBDataBuffer():_data(0),_length(0){
}

BBDataBuffer::~BBDataBuffer(){
	if( _data ) free( _data );
}

bool BBDataBuffer::_New( int length,void *data ){
	if( _data ) return false;
	if( !data ) data=malloc( length );
	_data=(signed char*)data;
	_length=length;
	return true;
}

bool BBDataBuffer::_Load( String path ){
	if( _data ) return false;
	
	_data=(signed char*)BBGame::Game()->LoadData( path,&_length );
	if( !_data ) return false;
	
	return true;
}

void BBDataBuffer::_LoadAsync( String path,BBThread *thread ){
	if( _Load( path ) ) thread->SetResult( this );
}

void BBDataBuffer::Discard(){
	if( !_data ) return;
	free( _data );
	_data=0;
	_length=0;
}


// ***** stream.h *****

class BBStream : public Object{
public:

	virtual int Eof(){
		return 0;
	}

	virtual void Close(){
	}

	virtual int Length(){
		return 0;
	}
	
	virtual int Position(){
		return 0;
	}
	
	virtual int Seek( int position ){
		return 0;
	}
	
	virtual int Read( BBDataBuffer *buffer,int offset,int count ){
		return 0;
	}

	virtual int Write( BBDataBuffer *buffer,int offset,int count ){
		return 0;
	}
};

// ***** stream.cpp *****


// ***** filestream.h *****

class BBFileStream : public BBStream{
public:

	BBFileStream();
	~BBFileStream();

	void Close();
	int Eof();
	int Length();
	int Position();
	int Seek( int position );
	int Read( BBDataBuffer *buffer,int offset,int count );
	int Write( BBDataBuffer *buffer,int offset,int count );

	bool Open( String path,String mode );
	
private:
	FILE *_file;
	int _position;
	int _length;
};

// ***** filestream.cpp *****

BBFileStream::BBFileStream():_file(0),_position(0),_length(0){
}

BBFileStream::~BBFileStream(){
	if( _file ) fclose( _file );
}

bool BBFileStream::Open( String path,String mode ){
	if( _file ) return false;

	String fmode;	
	if( mode=="r" ){
		fmode="rb";
	}else if( mode=="w" ){
		fmode="wb";
	}else if( mode=="u" ){
		fmode="rb+";
	}else{
		return false;
	}

	_file=BBGame::Game()->OpenFile( path,fmode );
	if( !_file && mode=="u" ) _file=BBGame::Game()->OpenFile( path,"wb+" );
	if( !_file ) return false;
	
	fseek( _file,0,SEEK_END );
	_length=ftell( _file );
	fseek( _file,0,SEEK_SET );
	_position=0;
	
	return true;
}

void BBFileStream::Close(){
	if( !_file ) return;
	
	fclose( _file );
	_file=0;
	_position=0;
	_length=0;
}

int BBFileStream::Eof(){
	if( !_file ) return -1;
	
	return _position==_length;
}

int BBFileStream::Length(){
	return _length;
}

int BBFileStream::Position(){
	return _position;
}

int BBFileStream::Seek( int position ){
	if( !_file ) return 0;
	
	fseek( _file,position,SEEK_SET );
	_position=ftell( _file );
	return _position;
}

int BBFileStream::Read( BBDataBuffer *buffer,int offset,int count ){
	if( !_file ) return 0;
	
	int n=fread( buffer->WritePointer(offset),1,count,_file );
	_position+=n;
	return n;
}

int BBFileStream::Write( BBDataBuffer *buffer,int offset,int count ){
	if( !_file ) return 0;
	
	int n=fwrite( buffer->ReadPointer(offset),1,count,_file );
	_position+=n;
	if( _position>_length ) _length=_position;
	return n;
}


#if !WINDOWS_8

// ***** socket.h *****

#if WINDOWS_PHONE_8

#include <Winsock2.h>

typedef int socklen_t;

#elif _WIN32

#include <winsock.h>

typedef int socklen_t;

#else

#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define closesocket close
#define ioctlsocket ioctl

#endif

class BBSocketAddress : public Object{
public:
	sockaddr_in _sa;
	
	BBSocketAddress();
	
	void Set( String host,int port );
	void Set( BBSocketAddress *address );
	void Set( const sockaddr_in &sa );
	
	String Host(){ Validate();return _host; }
	int Port(){ Validate();return _port; }
	
private:
	bool _valid;
	String _host;
	int _port;
	
	void Validate();
};

class BBSocket : public Object{
public:
	enum{
		PROTOCOL_CLIENT=1,
		PROTOCOL_SERVER=2,
		PROTOCOL_DATAGRAM=3
	};
	
	BBSocket();
	BBSocket( int sock );
	~BBSocket();
	
	bool Open( int protocol );
	void Close();
	
	bool Bind( String host,int port );
	bool Connect( String host,int port );
	bool Listen( int backlog );
	bool Accept();
	BBSocket *Accepted();

	int Send( BBDataBuffer *data,int offset,int count );
	int Receive( BBDataBuffer *data,int offset,int count );

	int SendTo( BBDataBuffer *data,int offset,int count,BBSocketAddress *address );
	int ReceiveFrom( BBDataBuffer *data,int offset,int count,BBSocketAddress *address );
	
	void GetLocalAddress( BBSocketAddress *address );
	void GetRemoteAddress( BBSocketAddress *address );
	
	static void InitSockets();
	
protected:
	int _sock;
	int _proto;
	int _accepted;
};

// ***** socket.cpp *****

static void setsockaddr( sockaddr_in *sa,String host,int port ){
	memset( sa,0,sizeof(*sa) );
	sa->sin_family=AF_INET;
	sa->sin_port=htons( port );
	sa->sin_addr.s_addr=htonl( INADDR_ANY );
	
	if( host.Length() && host.Length()<1024 ){
		char buf[1024];
		for( int i=0;i<host.Length();++i ) buf[i]=host[i];
		buf[host.Length()]=0;
		if( hostent *host=gethostbyname( buf ) ){
			if( char *hostip=inet_ntoa(*(in_addr *)*host->h_addr_list) ){
				sa->sin_addr.s_addr=inet_addr( hostip );
			}
		}
	}
}

void BBSocket::InitSockets(){
#if _WIN32
	static bool started;
	if( !started ){
		WSADATA ws;
		WSAStartup( 0x101,&ws );
		started=true;
	}
#endif
}

BBSocketAddress::BBSocketAddress():_valid( false ){
	BBSocket::InitSockets();
	memset( &_sa,0,sizeof(_sa) );
	_sa.sin_family=AF_INET;
}

void BBSocketAddress::Set( String host,int port ){
	setsockaddr( &_sa,host,port );
	_valid=false;
}

void BBSocketAddress::Set( BBSocketAddress *address ){
	_sa=address->_sa;
	_valid=false;
}

void BBSocketAddress::Set( const sockaddr_in &sa ){
	_sa=sa;
	_valid=false;
}

void BBSocketAddress::Validate(){
	if( _valid ) return;
	_host=String( int(_sa.sin_addr.s_addr)&0xff )+"."+String( int(_sa.sin_addr.s_addr>>8)&0xff )+"."+String( int(_sa.sin_addr.s_addr>>16)&0xff )+"."+String( int(_sa.sin_addr.s_addr>>24)&0xff );
	_port=htons( _sa.sin_port );
	_valid=true;
}

BBSocket::BBSocket():_sock( -1 ){
	BBSocket::InitSockets();
}

BBSocket::BBSocket( int sock ):_sock( sock ){
}

BBSocket::~BBSocket(){

	if( _sock>=0 ) closesocket( _sock );
}

bool BBSocket::Open( int proto ){

	if( _sock>=0 ) return false;
	
	switch( proto ){
	case PROTOCOL_CLIENT:
	case PROTOCOL_SERVER:
		_sock=socket( AF_INET,SOCK_STREAM,IPPROTO_TCP );
		if( _sock>=0 ){
			//nodelay
			int nodelay=1;
			setsockopt( _sock,IPPROTO_TCP,TCP_NODELAY,(const char*)&nodelay,sizeof(nodelay) );
	
			//Do this on Mac so server ports can be quickly reused...
			#if __APPLE__
			int flag=1;
			setsockopt( _sock,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag) );
			#endif
		}
		break;
	case PROTOCOL_DATAGRAM:
		_sock=socket( AF_INET,SOCK_DGRAM,IPPROTO_UDP );
		break;
	}
	
	if( _sock<0 ) return false;
	
	_proto=proto;
	return true;
}

void BBSocket::Close(){
	if( _sock<0 ) return;
	closesocket( _sock );
	_sock=-1;
}

void BBSocket::GetLocalAddress( BBSocketAddress *address ){
	sockaddr_in sa;
	memset( &sa,0,sizeof(sa) );
	sa.sin_family=AF_INET;
	socklen_t size=sizeof(sa);
	if( _sock>=0 ) getsockname( _sock,(sockaddr*)&sa,&size );
	address->Set( sa );
}

void BBSocket::GetRemoteAddress( BBSocketAddress *address ){
	sockaddr_in sa;
	memset( &sa,0,sizeof(sa) );
	sa.sin_family=AF_INET;
	socklen_t size=sizeof(sa);
	if( _sock>=0 ) getpeername( _sock,(sockaddr*)&sa,&size );
	address->Set( sa );
}

bool BBSocket::Bind( String host,int port ){

	sockaddr_in sa;
	setsockaddr( &sa,host,port );
	
	return bind( _sock,(sockaddr*)&sa,sizeof(sa) )>=0;
}

bool BBSocket::Connect( String host,int port ){

	sockaddr_in sa;
	setsockaddr( &sa,host,port );
	
	return connect( _sock,(sockaddr*)&sa,sizeof(sa) )>=0;
}

bool BBSocket::Listen( int backlog ){
	return listen( _sock,backlog )>=0;
}

bool BBSocket::Accept(){
	_accepted=accept( _sock,0,0 );
	return _accepted>=0;
}

BBSocket *BBSocket::Accepted(){
	if( _accepted>=0 ) return new BBSocket( _accepted );
	return 0;
}

int BBSocket::Send( BBDataBuffer *data,int offset,int count ){
	return send( _sock,(const char*)data->ReadPointer(offset),count,0 );
}

int BBSocket::Receive( BBDataBuffer *data,int offset,int count ){
	return recv( _sock,(char*)data->WritePointer( offset ),count,0 );
}

int BBSocket::SendTo( BBDataBuffer *data,int offset,int count,BBSocketAddress *address ){
	return sendto( _sock,(const char*)data->ReadPointer(offset),count,0,(sockaddr*)&address->_sa,sizeof(address->_sa) );
}

int BBSocket::ReceiveFrom( BBDataBuffer *data,int offset,int count,BBSocketAddress *address ){
	sockaddr_in sa;
	socklen_t size=sizeof(sa);
	memset( &sa,0,size );
	int n=recvfrom( _sock,(char*)data->WritePointer( offset ),count,0,(sockaddr*)&sa,&size );
	address->Set( sa );
	return n;
}

#endif


// The gloriously *MAD* winrt version!

#if WINDOWS_8

// ***** socket_winrt.h *****

#include <map>

using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

class BBSocketAddress : public Object{
public:
	HostName ^hostname;
	Platform::String ^service;
	
	BBSocketAddress();

	void Set( BBSocketAddress *address );
	void Set( String host,int port );

	String Host();
	int Port();
	
	bool operator<( const BBSocketAddress &t )const;
};

class BBSocket : public Object{
public:

	enum{
		PROTOCOL_STREAM=1,
		PROTOCOL_SERVER=2,
		PROTOCOL_DATAGRAM=3
	};

	BBSocket();
	~BBSocket();
	
	bool Open( int protocol );
	bool Open( StreamSocket ^stream );
	void Close();
	
	bool Bind( String host,int port );
	bool Connect( String host,int port );
	bool Listen( int backlog );
	bool Accept();
	BBSocket *Accepted();

	int Send( BBDataBuffer *data,int offset,int count );
	int Receive( BBDataBuffer *data,int offset,int count );
	
	int SendTo( BBDataBuffer *data,int offset,int count,BBSocketAddress *address );
	int ReceiveFrom( BBDataBuffer *data,int offset,int count,BBSocketAddress *address );
	
	void GetLocalAddress( BBSocketAddress *address );
	void GetRemoteAddress( BBSocketAddress *address );

private:

	StreamSocket ^_stream;
	StreamSocketListener ^_server;
	DatagramSocket ^_datagram;
	
	HANDLE _revent;
	HANDLE _wevent;

	DataReader ^_reader;
	DataWriter ^_writer;
	
	AsyncOperationCompletedHandler<unsigned int> ^_recvhandler;
	AsyncOperationCompletedHandler<unsigned int> ^_sendhandler;
	AsyncOperationCompletedHandler<IOutputStream^> ^_getouthandler; 
	
	//for "server" sockets only
	StreamSocket ^_accepted;
	std::vector<StreamSocket^> _acceptQueue;
	int _acceptPut,_acceptGet;
	HANDLE _acceptSema;	

	//for "datagram" sockets only
	typedef DatagramSocketMessageReceivedEventArgs RecvArgs;
	std::vector<RecvArgs^> _recvQueue;
	int _recvPut,_recvGet;
	HANDLE _recvSema;
	
	//for "datagram" sendto
	std::map<BBSocketAddress,DataWriter^> _sendToMap;
	
	template<class X,class Y> struct Delegate{
		BBSocket *socket;
		void (BBSocket::*func)( X,Y );
		Delegate( BBSocket *socket,void (BBSocket::*func)( X,Y ) ):socket( socket ),func( func ){
		}
		void operator()( X x,Y y ){
			(socket->*func)( x,y );
		}
	};
	template<class X,class Y> friend struct Delegate;
	
	template<class X,class Y> Delegate<X,Y> MakeDelegate( void (BBSocket::*func)( X,Y ) ){
		return Delegate<X,Y>( this,func );
	}
	
	template<class X,class Y> TypedEventHandler<X,Y> ^CreateTypedEventHandler( void (BBSocket::*func)( X,Y ) ){
		return ref new TypedEventHandler<X,Y>( Delegate<X,Y>( this,func ) );
	}

	bool Wait( IAsyncAction ^action );
	
	void OnActionComplete( IAsyncAction ^action,AsyncStatus status );
	void OnSendComplete( IAsyncOperation<unsigned int> ^op,AsyncStatus status );
	void OnReceiveComplete( IAsyncOperation<unsigned int> ^op,AsyncStatus status );
	void OnConnectionReceived( StreamSocketListener ^listener,StreamSocketListenerConnectionReceivedEventArgs ^args );
	void OnMessageReceived( DatagramSocket ^socket,DatagramSocketMessageReceivedEventArgs ^args );
	void OnGetOutputStreamComplete( IAsyncOperation<IOutputStream^> ^op,AsyncStatus status );
};

// ***** socket_winrt.cpp *****

BBSocketAddress::BBSocketAddress():hostname( nullptr ),service( nullptr ){
}

void BBSocketAddress::Set( String host,int port ){
	HostName ^hostname=nullptr;
	if( host.Length() ) hostname=ref new HostName( host.ToWinRTString() );
	service=String( port ).ToWinRTString();
}

void BBSocketAddress::Set( BBSocketAddress *address ){
	hostname=address->hostname;
	service=address->service;
}

String BBSocketAddress::Host(){
	return hostname ? hostname->CanonicalName : "0.0.0.0";
}

int BBSocketAddress::Port(){
	return service ? String( service->Data(),service->Length() ).ToInt() : 0;
}

bool BBSocketAddress::operator<( const BBSocketAddress &t )const{
	if( hostname || t.hostname ){
		if( !hostname ) return true;
		if( !t.hostname ) return false;
		int n=HostName::Compare( hostname->CanonicalName,t.hostname->CanonicalName );
		if( n ) return n<0;
	}
	if( service || t.service ){
		if( !service ) return -1;
		if( !t.service ) return 1;
		int n=Platform::String::CompareOrdinal( service,t.service );
		if( n ) return n<0;
	}
	return false;
}

BBSocket::BBSocket(){

	_revent=CreateEventEx( 0,0,0,EVENT_ALL_ACCESS );
	_wevent=CreateEventEx( 0,0,0,EVENT_ALL_ACCESS );
	
	_recvSema=0;
	_acceptSema=0;
	
	_sendhandler=ref new AsyncOperationCompletedHandler<unsigned int>( MakeDelegate( &BBSocket::OnSendComplete ) );
	_recvhandler=ref new AsyncOperationCompletedHandler<unsigned int>( MakeDelegate( &BBSocket::OnReceiveComplete ) );
	_getouthandler=ref new AsyncOperationCompletedHandler<IOutputStream^>( MakeDelegate( &BBSocket::OnGetOutputStreamComplete ) );	
}

BBSocket::~BBSocket(){
	if( _revent ) CloseHandle( _revent );
	if( _wevent ) CloseHandle( _wevent );
	if( _recvSema ) CloseHandle( _recvSema );
	if( _acceptSema ) CloseHandle( _acceptSema );
}

void BBSocket::OnActionComplete( IAsyncAction ^action,AsyncStatus status ){
	SetEvent( _revent );
}

bool BBSocket::Wait( IAsyncAction ^action ){
	action->Completed=ref new AsyncActionCompletedHandler( MakeDelegate( &BBSocket::OnActionComplete ) );
	if( WaitForSingleObjectEx( _revent,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return false;
	return action->Status==AsyncStatus::Completed;
}

bool BBSocket::Open( int protocol ){

	switch( protocol ){
	case PROTOCOL_STREAM:
		_stream=ref new StreamSocket();
		return true;
	case PROTOCOL_SERVER:
		_acceptGet=_acceptPut=0;
		_acceptQueue.resize( 256 );
		_acceptSema=CreateSemaphoreEx( 0,0,256,0,0,EVENT_ALL_ACCESS );
		_server=ref new StreamSocketListener();
		_server->ConnectionReceived+=CreateTypedEventHandler( &BBSocket::OnConnectionReceived );
		return true;
	case PROTOCOL_DATAGRAM:
		_recvGet=_recvPut=0;
		_recvQueue.resize( 256 );
		_recvSema=CreateSemaphoreEx( 0,0,256,0,0,EVENT_ALL_ACCESS );
		_datagram=ref new DatagramSocket();
		_datagram->MessageReceived+=CreateTypedEventHandler( &BBSocket::OnMessageReceived );
		return true;
	}

	return false;
}

bool BBSocket::Open( StreamSocket ^stream ){

	_stream=stream;
	
	_reader=ref new DataReader( _stream->InputStream );
	_reader->InputStreamOptions=InputStreamOptions::Partial;
	
	_writer=ref new DataWriter( _stream->OutputStream );
	
	return true;
}

void BBSocket::Close(){
	if( _stream ) delete _stream;
	if( _server ) delete _server;
	if( _datagram ) delete _datagram;
	_stream=nullptr;
	_server=nullptr;
	_datagram=nullptr;
}

bool BBSocket::Bind( String host,int port ){

	HostName ^hostname=nullptr;
	if( host.Length() ) hostname=ref new HostName( host.ToWinRTString() );
	auto service=(port ? String( port ) : String()).ToWinRTString();

	if( _stream ){
//		return Wait( _stream->BindEndpointAsync( hostname,service ) );
	}else if( _server ){
		return Wait( _server->BindEndpointAsync( hostname,service ) );
	}else if( _datagram ){
		return Wait( _datagram->BindEndpointAsync( hostname,service ) );
	}

	return false;
}

bool BBSocket::Listen( int backlog ){
	return _server!=nullptr;
}

bool BBSocket::Accept(){
	if( WaitForSingleObjectEx( _acceptSema,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return false;
	_accepted=_acceptQueue[_acceptGet & 255];
	_acceptQueue[_acceptGet++ & 255]=nullptr;
	return true;
}

BBSocket *BBSocket::Accepted(){
	BBSocket *socket=new BBSocket();
	if( socket->Open( _accepted ) ) return socket;
	return 0;
}

void BBSocket::OnConnectionReceived( StreamSocketListener ^listener,StreamSocketListenerConnectionReceivedEventArgs ^args ){

	_acceptQueue[_acceptPut++ & 255]=args->Socket;
	ReleaseSemaphore( _acceptSema,1,0 );
}

void BBSocket::OnMessageReceived( DatagramSocket ^socket,DatagramSocketMessageReceivedEventArgs ^args ){

	_recvQueue[_recvPut++ & 255]=args;
	ReleaseSemaphore( _recvSema,1,0 );
}

bool BBSocket::Connect( String host,int port ){

	auto hostname=ref new HostName( host.ToWinRTString() );
	auto service=String( port ).ToWinRTString();

	if( _stream ){

		if( !Wait( _stream->ConnectAsync( hostname,service ) ) ) return false;
		
		_reader=ref new DataReader( _stream->InputStream );
		_reader->InputStreamOptions=InputStreamOptions::Partial;

		_writer=ref new DataWriter( _stream->OutputStream );
	
		return true;
		
	}else if( _datagram ) {
	
		if( !Wait( _datagram->ConnectAsync( hostname,service ) ) ) return false;
		
		_writer=ref new DataWriter( _datagram->OutputStream );
		
		return true;
	}
}

int BBSocket::Send( BBDataBuffer *data,int offset,int count ){

	if( !_writer ) return 0;

	const unsigned char *p=(const unsigned char*)data->ReadPointer( offset );
	
	_writer->WriteBytes( Platform::ArrayReference<uint8>( (uint8*)p,count ) );
	auto op=_writer->StoreAsync();
	op->Completed=_sendhandler;
	
	if( WaitForSingleObjectEx( _wevent,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return 0;

//	if( op->Status!=AsyncStatus::Completed ) return 0;
	
	return count;
}

void BBSocket::OnSendComplete( IAsyncOperation<unsigned int> ^op,AsyncStatus status ){

	SetEvent( _wevent );
}

int BBSocket::Receive( BBDataBuffer *data,int offset,int count ){

	if( _stream ){
	
		auto op=_reader->LoadAsync( count );
		op->Completed=_recvhandler;
	
		if( WaitForSingleObjectEx( _revent,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return 0;
		
	//	if( op->Status!=AsyncStatus::Completed ) return 0;
		
		int n=_reader->UnconsumedBufferLength;
			
		_reader->ReadBytes( Platform::ArrayReference<uint8>( (uint8*)data->WritePointer( offset ),n ) );
	
		return n;
		
	}else if( _datagram ){

		if( WaitForSingleObjectEx( _recvSema,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return 0;
		
		auto recvArgs=_recvQueue[_recvGet & 255];
		_recvQueue[_recvGet++ & 255]=nullptr;
		
		auto reader=recvArgs->GetDataReader();
		int n=reader->UnconsumedBufferLength;
		if( n>count ) n=count;

		reader->ReadBytes( Platform::ArrayReference<uint8>( (uint8*)data->WritePointer( offset ),n ) );
		
		return n;
	}
	return 0;
}

void BBSocket::OnReceiveComplete( IAsyncOperation<unsigned int> ^op,AsyncStatus status ){

	SetEvent( _revent );
}

int BBSocket::SendTo( BBDataBuffer *data,int offset,int count,BBSocketAddress *address ){

	auto it=_sendToMap.find( *address );
	
	if( it==_sendToMap.end() ){
	
		auto op=_datagram->GetOutputStreamAsync( address->hostname,address->service );
		op->Completed=_getouthandler;
		
		if( WaitForSingleObjectEx( _wevent,INFINITE,FALSE )!=WAIT_OBJECT_0 || op->Status!=AsyncStatus::Completed ){
			bbPrint( "GetOutputStream failed" );
			return 0;
		}	
		it=_sendToMap.insert( std::make_pair( *address,ref new DataWriter( op->GetResults() ) ) ).first;
	}

	auto writer=it->second;

	writer->WriteBytes( Platform::ArrayReference<uint8>( (uint8*)data->ReadPointer( offset ),count ) );
	auto op=writer->StoreAsync();
	op->Completed=_sendhandler;
	
	if( WaitForSingleObjectEx( _wevent,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return 0;

//	if( op->Status!=AsyncStatus::Completed ) return 0;
	
	return count;
}

void BBSocket::OnGetOutputStreamComplete( IAsyncOperation<IOutputStream^> ^op,AsyncStatus status ){

	SetEvent( _wevent );
}

int BBSocket::ReceiveFrom( BBDataBuffer *data,int offset,int count,BBSocketAddress *address ){

	if( !_datagram ) return 0;
	
	if( WaitForSingleObjectEx( _recvSema,INFINITE,FALSE )!=WAIT_OBJECT_0 ) return 0;
	
	auto recvArgs=_recvQueue[_recvGet & 255];
	_recvQueue[_recvGet++ & 255]=nullptr;
	
	auto reader=recvArgs->GetDataReader();
	int n=reader->UnconsumedBufferLength;
	if( n>count ) n=count;

	reader->ReadBytes( Platform::ArrayReference<uint8>( (uint8*)data->WritePointer( offset ),n ) );

	address->hostname=recvArgs->RemoteAddress;
	address->service=recvArgs->RemotePort;
	
	return n;
}
	
void BBSocket::GetLocalAddress( BBSocketAddress *address ){
	if( _stream ){
		address->hostname=_stream->Information->LocalAddress;
		address->service=_stream->Information->LocalPort;
	}else if( _server ){
		address->hostname=nullptr;
		address->service=_server->Information->LocalPort;
	}else if( _datagram ){
		address->hostname=_datagram->Information->LocalAddress;
		address->service=_datagram->Information->LocalPort;
	}
}

void BBSocket::GetRemoteAddress( BBSocketAddress *address ){
	if( _stream ){
		address->hostname=_stream->Information->RemoteAddress;
		address->service=_stream->Information->RemotePort;
	}else if( _server ){
		address->hostname=nullptr;
		address->service=nullptr;
	}else if( _datagram ){
		address->hostname=_datagram->Information->RemoteAddress;
		address->service=_datagram->Information->RemotePort;
	}
}

#endif

class c_App;
class c_DungeonGen;
class c_GameDelegate;
class c_Image;
class c_GraphicsContext;
class c_Frame;
class c_InputDevice;
class c_JoyState;
class c_DisplayMode;
class c_Map;
class c_IntMap;
class c_Stack;
class c_Node;
class c_BBGameEvent;
class c_RoomMap;
class c_Room;
class c_List;
class c_Node2;
class c_HeadNode;
class c_Enumerator;
class c_App : public Object{
	public:
	c_App();
	c_App* m_new();
	int p_OnResize();
	virtual int p_OnCreate();
	int p_OnSuspend();
	int p_OnResume();
	virtual int p_OnUpdate();
	int p_OnLoading();
	virtual int p_OnRender();
	int p_OnClose();
	int p_OnBack();
	void mark();
	String debug();
};
String dbg_type(c_App**p){return "App";}
class c_DungeonGen : public c_App{
	public:
	c_RoomMap* m_Room1;
	String m_GameState;
	c_DungeonGen();
	c_DungeonGen* m_new();
	int p_OnCreate();
	int p_OnUpdate();
	int p_OnRender();
	void mark();
	String debug();
};
String dbg_type(c_DungeonGen**p){return "DungeonGen";}
extern c_App* bb_app__app;
class c_GameDelegate : public BBGameDelegate{
	public:
	gxtkGraphics* m__graphics;
	gxtkAudio* m__audio;
	c_InputDevice* m__input;
	c_GameDelegate();
	c_GameDelegate* m_new();
	void StartGame();
	void SuspendGame();
	void ResumeGame();
	void UpdateGame();
	void RenderGame();
	void KeyEvent(int,int);
	void MouseEvent(int,int,Float,Float);
	void TouchEvent(int,int,Float,Float);
	void MotionEvent(int,int,Float,Float,Float);
	void DiscardGraphics();
	void mark();
	String debug();
};
String dbg_type(c_GameDelegate**p){return "GameDelegate";}
extern c_GameDelegate* bb_app__delegate;
extern BBGame* bb_app__game;
extern c_DungeonGen* bb_LayeredLevel_Prototype;
int bbMain();
extern gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics*);
class c_Image : public Object{
	public:
	gxtkSurface* m_surface;
	int m_width;
	int m_height;
	Array<c_Frame* > m_frames;
	int m_flags;
	Float m_tx;
	Float m_ty;
	c_Image* m_source;
	c_Image();
	static int m_DefaultFlags;
	c_Image* m_new();
	int p_SetHandle(Float,Float);
	int p_ApplyFlags(int);
	c_Image* p_Init(gxtkSurface*,int,int);
	c_Image* p_Init2(gxtkSurface*,int,int,int,int,int,int,c_Image*,int,int,int,int);
	int p_Width();
	int p_Height();
	int p_Frames();
	c_Image* p_GrabImage(int,int,int,int,int,int);
	void mark();
	String debug();
};
String dbg_type(c_Image**p){return "Image";}
class c_GraphicsContext : public Object{
	public:
	c_Image* m_defaultFont;
	c_Image* m_font;
	int m_firstChar;
	int m_matrixSp;
	Float m_ix;
	Float m_iy;
	Float m_jx;
	Float m_jy;
	Float m_tx;
	Float m_ty;
	int m_tformed;
	int m_matDirty;
	Float m_color_r;
	Float m_color_g;
	Float m_color_b;
	Float m_alpha;
	int m_blend;
	Float m_scissor_x;
	Float m_scissor_y;
	Float m_scissor_width;
	Float m_scissor_height;
	Array<Float > m_matrixStack;
	c_GraphicsContext();
	c_GraphicsContext* m_new();
	int p_Validate();
	void mark();
	String debug();
};
String dbg_type(c_GraphicsContext**p){return "GraphicsContext";}
extern c_GraphicsContext* bb_graphics_context;
String bb_data_FixDataPath(String);
class c_Frame : public Object{
	public:
	int m_x;
	int m_y;
	c_Frame();
	c_Frame* m_new(int,int);
	c_Frame* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Frame**p){return "Frame";}
c_Image* bb_graphics_LoadImage(String,int,int);
c_Image* bb_graphics_LoadImage2(String,int,int,int,int);
int bb_graphics_SetFont(c_Image*,int);
extern gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio*);
class c_InputDevice : public Object{
	public:
	Array<c_JoyState* > m__joyStates;
	Array<bool > m__keyDown;
	int m__keyHitPut;
	Array<int > m__keyHitQueue;
	Array<int > m__keyHit;
	int m__charGet;
	int m__charPut;
	Array<int > m__charQueue;
	Float m__mouseX;
	Float m__mouseY;
	Array<Float > m__touchX;
	Array<Float > m__touchY;
	Float m__accelX;
	Float m__accelY;
	Float m__accelZ;
	c_InputDevice();
	c_InputDevice* m_new();
	void p_PutKeyHit(int);
	void p_BeginUpdate();
	void p_EndUpdate();
	void p_KeyEvent(int,int);
	void p_MouseEvent(int,int,Float,Float);
	void p_TouchEvent(int,int,Float,Float);
	void p_MotionEvent(int,int,Float,Float,Float);
	int p_KeyHit(int);
	void mark();
	String debug();
};
String dbg_type(c_InputDevice**p){return "InputDevice";}
class c_JoyState : public Object{
	public:
	Array<Float > m_joyx;
	Array<Float > m_joyy;
	Array<Float > m_joyz;
	Array<bool > m_buttons;
	c_JoyState();
	c_JoyState* m_new();
	void mark();
	String debug();
};
String dbg_type(c_JoyState**p){return "JoyState";}
extern c_InputDevice* bb_input_device;
int bb_input_SetInputDevice(c_InputDevice*);
extern int bb_app__devWidth;
extern int bb_app__devHeight;
void bb_app_ValidateDeviceWindow(bool);
class c_DisplayMode : public Object{
	public:
	int m__width;
	int m__height;
	c_DisplayMode();
	c_DisplayMode* m_new(int,int);
	c_DisplayMode* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_DisplayMode**p){return "DisplayMode";}
class c_Map : public Object{
	public:
	c_Node* m_root;
	c_Map();
	c_Map* m_new();
	virtual int p_Compare(int,int)=0;
	c_Node* p_FindNode(int);
	bool p_Contains(int);
	int p_RotateLeft(c_Node*);
	int p_RotateRight(c_Node*);
	int p_InsertFixup(c_Node*);
	bool p_Set(int,c_DisplayMode*);
	bool p_Insert(int,c_DisplayMode*);
	void mark();
	String debug();
};
String dbg_type(c_Map**p){return "Map";}
class c_IntMap : public c_Map{
	public:
	c_IntMap();
	c_IntMap* m_new();
	int p_Compare(int,int);
	void mark();
	String debug();
};
String dbg_type(c_IntMap**p){return "IntMap";}
class c_Stack : public Object{
	public:
	Array<c_DisplayMode* > m_data;
	int m_length;
	c_Stack();
	c_Stack* m_new();
	c_Stack* m_new2(Array<c_DisplayMode* >);
	void p_Push(c_DisplayMode*);
	void p_Push2(Array<c_DisplayMode* >,int,int);
	void p_Push3(Array<c_DisplayMode* >,int);
	Array<c_DisplayMode* > p_ToArray();
	void mark();
	String debug();
};
String dbg_type(c_Stack**p){return "Stack";}
class c_Node : public Object{
	public:
	int m_key;
	c_Node* m_right;
	c_Node* m_left;
	c_DisplayMode* m_value;
	int m_color;
	c_Node* m_parent;
	c_Node();
	c_Node* m_new(int,c_DisplayMode*,int,c_Node*);
	c_Node* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Node**p){return "Node";}
extern Array<c_DisplayMode* > bb_app__displayModes;
extern c_DisplayMode* bb_app__desktopMode;
int bb_app_DeviceWidth();
int bb_app_DeviceHeight();
void bb_app_EnumDisplayModes();
extern gxtkGraphics* bb_graphics_renderDevice;
int bb_graphics_SetMatrix(Float,Float,Float,Float,Float,Float);
int bb_graphics_SetMatrix2(Array<Float >);
int bb_graphics_SetColor(Float,Float,Float);
int bb_graphics_SetAlpha(Float);
int bb_graphics_SetBlend(int);
int bb_graphics_SetScissor(Float,Float,Float,Float);
int bb_graphics_BeginRender();
int bb_graphics_EndRender();
class c_BBGameEvent : public Object{
	public:
	c_BBGameEvent();
	void mark();
	String debug();
};
String dbg_type(c_BBGameEvent**p){return "BBGameEvent";}
void bb_app_EndApp();
class c_RoomMap : public Object{
	public:
	int m_MapWidth;
	int m_MapHeight;
	Array<Array<c_Room* > > m_Map;
	c_Room* m_currentRoom;
	int m_roomNum;
	int m_MapPSize;
	int m_MapXOffset;
	c_RoomMap();
	c_RoomMap* m_new();
	int p_Build();
	int p_Reset();
	int p_DrawRoomFloor();
	int p_DrawRoomWalls();
	int p_DrawMap();
	void mark();
	String debug();
};
String dbg_type(c_RoomMap**p){return "RoomMap";}
extern int bb_app__updateRate;
void bb_app_SetUpdateRate(int);
int bb_input_KeyHit(int);
int bb_app_Millisecs();
extern int bb_random_Seed;
class c_Room : public Object{
	public:
	int m_x;
	int m_y;
	int m_Type;
	Array<Array<int > > m_WallLayout;
	Array<Array<int > > m_FloorLayout;
	Array<Array<int > > m_CollisionArray;
	int m_Neighbours;
	String m_nDoor;
	String m_sDoor;
	String m_wDoor;
	String m_eDoor;
	int m_RoomSize32;
	c_Image* m_RoomTiles;
	Array<String > m_DoorArray;
	c_Room();
	c_Room* m_new(int,int,int);
	c_Room* m_new2();
	int p_UpdateType(int);
	int p_UpdateNeighbours(int);
	int p_GetNeighbours();
	int p_GetType();
	int p_GetX();
	int p_GetY();
	int p_SetnDoor(String);
	int p_SetsDoor(String);
	int p_SetwDoor(String);
	int p_SeteDoor(String);
	int p_Reset();
	int p_DrawFloor();
	int p_DrawWalls();
	String p_GetDoors();
	void mark();
	String debug();
};
String dbg_type(c_Room**p){return "Room";}
Float bb_random_Rnd();
Float bb_random_Rnd2(Float,Float);
Float bb_random_Rnd3(Float);
class c_List : public Object{
	public:
	c_Node2* m__head;
	c_List();
	c_List* m_new();
	c_Node2* p_AddLast(c_Room*);
	c_List* m_new2(Array<c_Room* >);
	int p_Count();
	c_Enumerator* p_ObjectEnumerator();
	Array<c_Room* > p_ToArray();
	void mark();
	String debug();
};
String dbg_type(c_List**p){return "List";}
class c_Node2 : public Object{
	public:
	c_Node2* m__succ;
	c_Node2* m__pred;
	c_Room* m__data;
	c_Node2();
	c_Node2* m_new(c_Node2*,c_Node2*,c_Room*);
	c_Node2* m_new2();
	void mark();
	String debug();
};
String dbg_type(c_Node2**p){return "Node";}
class c_HeadNode : public c_Node2{
	public:
	c_HeadNode();
	c_HeadNode* m_new();
	void mark();
	String debug();
};
String dbg_type(c_HeadNode**p){return "HeadNode";}
class c_Enumerator : public Object{
	public:
	c_List* m__list;
	c_Node2* m__curr;
	c_Enumerator();
	c_Enumerator* m_new(c_List*);
	c_Enumerator* m_new2();
	bool p_HasNext();
	c_Room* p_NextObject();
	void mark();
	String debug();
};
String dbg_type(c_Enumerator**p){return "Enumerator";}
int bb_graphics_DebugRenderDevice();
int bb_graphics_Cls(Float,Float,Float);
int bb_graphics_DrawImage(c_Image*,Float,Float,int);
int bb_graphics_PushMatrix();
int bb_graphics_Transform(Float,Float,Float,Float,Float,Float);
int bb_graphics_Transform2(Array<Float >);
int bb_graphics_Translate(Float,Float);
int bb_graphics_Rotate(Float);
int bb_graphics_Scale(Float,Float);
int bb_graphics_PopMatrix();
int bb_graphics_DrawImage2(c_Image*,Float,Float,Float,Float,Float,int);
int bb_graphics_DrawText(String,Float,Float,Float,Float);
int bb_graphics_DrawRect(Float,Float,Float,Float);
int bb_LayeredLevel_DrawRoom(String,int,int,int,int);
c_App::c_App(){
}
c_App* c_App::m_new(){
	DBG_ENTER("App.new")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<152>");
	if((bb_app__app)!=0){
		DBG_BLOCK();
		bbError(String(L"App has already been created",28));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<153>");
	gc_assign(bb_app__app,this);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<154>");
	gc_assign(bb_app__delegate,(new c_GameDelegate)->m_new());
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<155>");
	bb_app__game->SetDelegate(bb_app__delegate);
	return this;
}
int c_App::p_OnResize(){
	DBG_ENTER("App.OnResize")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnCreate(){
	DBG_ENTER("App.OnCreate")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnSuspend(){
	DBG_ENTER("App.OnSuspend")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnResume(){
	DBG_ENTER("App.OnResume")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnUpdate(){
	DBG_ENTER("App.OnUpdate")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnLoading(){
	DBG_ENTER("App.OnLoading")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnRender(){
	DBG_ENTER("App.OnRender")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int c_App::p_OnClose(){
	DBG_ENTER("App.OnClose")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<177>");
	bb_app_EndApp();
	return 0;
}
int c_App::p_OnBack(){
	DBG_ENTER("App.OnBack")
	c_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<181>");
	p_OnClose();
	return 0;
}
void c_App::mark(){
	Object::mark();
}
String c_App::debug(){
	String t="(App)\n";
	return t;
}
c_DungeonGen::c_DungeonGen(){
	m_Room1=0;
	m_GameState=String(L"START",5);
}
c_DungeonGen* c_DungeonGen::m_new(){
	DBG_ENTER("DungeonGen.new")
	c_DungeonGen *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<13>");
	c_App::m_new();
	return this;
}
int c_DungeonGen::p_OnCreate(){
	DBG_ENTER("DungeonGen.OnCreate")
	c_DungeonGen *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<18>");
	gc_assign(m_Room1,(new c_RoomMap)->m_new());
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<19>");
	bb_app_SetUpdateRate(60);
	return 0;
}
int c_DungeonGen::p_OnUpdate(){
	DBG_ENTER("DungeonGen.OnUpdate")
	c_DungeonGen *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<23>");
	String t_1=m_GameState;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<24>");
	if(t_1==String(L"START",5)){
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<25>");
		if((bb_input_KeyHit(32))!=0){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<26>");
			bb_random_Seed=bb_app_Millisecs();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<27>");
			m_Room1->p_Build();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<28>");
			m_GameState=String(L"PLAYING",7);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<30>");
		if(t_1==String(L"PLAYING",7)){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<31>");
			if((bb_input_KeyHit(27))!=0){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<32>");
				m_Room1->p_Reset();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<33>");
				m_GameState=String(L"START",5);
			}
		}
	}
	return 0;
}
int c_DungeonGen::p_OnRender(){
	DBG_ENTER("DungeonGen.OnRender")
	c_DungeonGen *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<39>");
	String t_2=m_GameState;
	DBG_LOCAL(t_2,"2")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<40>");
	if(t_2==String(L"START",5)){
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<41>");
		bb_graphics_Cls(FLOAT(0.0),FLOAT(0.0),FLOAT(0.0));
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<42>");
		bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<43>");
		bb_graphics_DrawText(String(L"Press Space to Start",20),FLOAT(255.0),FLOAT(255.0),FLOAT(0.0),FLOAT(0.0));
	}else{
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<44>");
		if(t_2==String(L"PLAYING",7)){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<45>");
			bb_graphics_Cls(FLOAT(0.0),FLOAT(0.0),FLOAT(0.0));
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<47>");
			m_Room1->p_DrawRoomFloor();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<48>");
			m_Room1->p_DrawRoomWalls();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<49>");
			m_Room1->p_DrawMap();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<50>");
			bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<51>");
			bb_graphics_DrawText(String(L"Press ESC to reset",18),FLOAT(500.0),FLOAT(255.0),FLOAT(0.0),FLOAT(0.0));
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<52>");
			bb_graphics_DrawText(String(L"Seed:",5),FLOAT(500.0),FLOAT(300.0),FLOAT(0.0),FLOAT(0.0));
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<53>");
			bb_graphics_DrawText(String(bb_random_Seed),FLOAT(545.0),FLOAT(300.0),FLOAT(0.0),FLOAT(0.0));
		}
	}
	return 0;
}
void c_DungeonGen::mark(){
	c_App::mark();
	gc_mark_q(m_Room1);
}
String c_DungeonGen::debug(){
	String t="(DungeonGen)\n";
	t=c_App::debug()+t;
	t+=dbg_decl("Room1",&m_Room1);
	t+=dbg_decl("GameState",&m_GameState);
	return t;
}
c_App* bb_app__app;
c_GameDelegate::c_GameDelegate(){
	m__graphics=0;
	m__audio=0;
	m__input=0;
}
c_GameDelegate* c_GameDelegate::m_new(){
	DBG_ENTER("GameDelegate.new")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<65>");
	return this;
}
void c_GameDelegate::StartGame(){
	DBG_ENTER("GameDelegate.StartGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<75>");
	gc_assign(m__graphics,(new gxtkGraphics));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<76>");
	bb_graphics_SetGraphicsDevice(m__graphics);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<77>");
	bb_graphics_SetFont(0,32);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<79>");
	gc_assign(m__audio,(new gxtkAudio));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<80>");
	bb_audio_SetAudioDevice(m__audio);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<82>");
	gc_assign(m__input,(new c_InputDevice)->m_new());
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<83>");
	bb_input_SetInputDevice(m__input);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<85>");
	bb_app_ValidateDeviceWindow(false);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<87>");
	bb_app_EnumDisplayModes();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<89>");
	bb_app__app->p_OnCreate();
}
void c_GameDelegate::SuspendGame(){
	DBG_ENTER("GameDelegate.SuspendGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<93>");
	bb_app__app->p_OnSuspend();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<94>");
	m__audio->Suspend();
}
void c_GameDelegate::ResumeGame(){
	DBG_ENTER("GameDelegate.ResumeGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<98>");
	m__audio->Resume();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<99>");
	bb_app__app->p_OnResume();
}
void c_GameDelegate::UpdateGame(){
	DBG_ENTER("GameDelegate.UpdateGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<103>");
	bb_app_ValidateDeviceWindow(true);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<104>");
	m__input->p_BeginUpdate();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<105>");
	bb_app__app->p_OnUpdate();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<106>");
	m__input->p_EndUpdate();
}
void c_GameDelegate::RenderGame(){
	DBG_ENTER("GameDelegate.RenderGame")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<110>");
	bb_app_ValidateDeviceWindow(true);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<111>");
	int t_mode=m__graphics->BeginRender();
	DBG_LOCAL(t_mode,"mode")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<112>");
	if((t_mode)!=0){
		DBG_BLOCK();
		bb_graphics_BeginRender();
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<113>");
	if(t_mode==2){
		DBG_BLOCK();
		bb_app__app->p_OnLoading();
	}else{
		DBG_BLOCK();
		bb_app__app->p_OnRender();
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<114>");
	if((t_mode)!=0){
		DBG_BLOCK();
		bb_graphics_EndRender();
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<115>");
	m__graphics->EndRender();
}
void c_GameDelegate::KeyEvent(int t_event,int t_data){
	DBG_ENTER("GameDelegate.KeyEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<119>");
	m__input->p_KeyEvent(t_event,t_data);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<120>");
	if(t_event!=1){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<121>");
	int t_1=t_data;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<122>");
	if(t_1==432){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<123>");
		bb_app__app->p_OnClose();
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<124>");
		if(t_1==416){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<125>");
			bb_app__app->p_OnBack();
		}
	}
}
void c_GameDelegate::MouseEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("GameDelegate.MouseEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<130>");
	m__input->p_MouseEvent(t_event,t_data,t_x,t_y);
}
void c_GameDelegate::TouchEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("GameDelegate.TouchEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<134>");
	m__input->p_TouchEvent(t_event,t_data,t_x,t_y);
}
void c_GameDelegate::MotionEvent(int t_event,int t_data,Float t_x,Float t_y,Float t_z){
	DBG_ENTER("GameDelegate.MotionEvent")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<138>");
	m__input->p_MotionEvent(t_event,t_data,t_x,t_y,t_z);
}
void c_GameDelegate::DiscardGraphics(){
	DBG_ENTER("GameDelegate.DiscardGraphics")
	c_GameDelegate *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<142>");
	m__graphics->DiscardGraphics();
}
void c_GameDelegate::mark(){
	BBGameDelegate::mark();
	gc_mark_q(m__graphics);
	gc_mark_q(m__audio);
	gc_mark_q(m__input);
}
String c_GameDelegate::debug(){
	String t="(GameDelegate)\n";
	t+=dbg_decl("_graphics",&m__graphics);
	t+=dbg_decl("_audio",&m__audio);
	t+=dbg_decl("_input",&m__input);
	return t;
}
c_GameDelegate* bb_app__delegate;
BBGame* bb_app__game;
c_DungeonGen* bb_LayeredLevel_Prototype;
int bbMain(){
	DBG_ENTER("Main")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<8>");
	gc_assign(bb_LayeredLevel_Prototype,(new c_DungeonGen)->m_new());
	return 0;
}
gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics* t_dev){
	DBG_ENTER("SetGraphicsDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<63>");
	gc_assign(bb_graphics_device,t_dev);
	return 0;
}
c_Image::c_Image(){
	m_surface=0;
	m_width=0;
	m_height=0;
	m_frames=Array<c_Frame* >();
	m_flags=0;
	m_tx=FLOAT(.0);
	m_ty=FLOAT(.0);
	m_source=0;
}
int c_Image::m_DefaultFlags;
c_Image* c_Image::m_new(){
	DBG_ENTER("Image.new")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<70>");
	return this;
}
int c_Image::p_SetHandle(Float t_tx,Float t_ty){
	DBG_ENTER("Image.SetHandle")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<114>");
	this->m_tx=t_tx;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<115>");
	this->m_ty=t_ty;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<116>");
	this->m_flags=this->m_flags&-2;
	return 0;
}
int c_Image::p_ApplyFlags(int t_iflags){
	DBG_ENTER("Image.ApplyFlags")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<192>");
	m_flags=t_iflags;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<194>");
	if((m_flags&2)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<195>");
		Array<c_Frame* > t_=m_frames;
		int t_2=0;
		while(t_2<t_.Length()){
			DBG_BLOCK();
			c_Frame* t_f=t_.At(t_2);
			t_2=t_2+1;
			DBG_LOCAL(t_f,"f")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<196>");
			t_f->m_x+=1;
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<198>");
		m_width-=2;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<201>");
	if((m_flags&4)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<202>");
		Array<c_Frame* > t_3=m_frames;
		int t_4=0;
		while(t_4<t_3.Length()){
			DBG_BLOCK();
			c_Frame* t_f2=t_3.At(t_4);
			t_4=t_4+1;
			DBG_LOCAL(t_f2,"f")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<203>");
			t_f2->m_y+=1;
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<205>");
		m_height-=2;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<208>");
	if((m_flags&1)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<209>");
		p_SetHandle(Float(m_width)/FLOAT(2.0),Float(m_height)/FLOAT(2.0));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<212>");
	if(m_frames.Length()==1 && m_frames.At(0)->m_x==0 && m_frames.At(0)->m_y==0 && m_width==m_surface->Width() && m_height==m_surface->Height()){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<213>");
		m_flags|=65536;
	}
	return 0;
}
c_Image* c_Image::p_Init(gxtkSurface* t_surf,int t_nframes,int t_iflags){
	DBG_ENTER("Image.Init")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_surf,"surf")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<150>");
	gc_assign(m_surface,t_surf);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<152>");
	m_width=m_surface->Width()/t_nframes;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<153>");
	m_height=m_surface->Height();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<155>");
	gc_assign(m_frames,Array<c_Frame* >(t_nframes));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<156>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<157>");
		gc_assign(m_frames.At(t_i),(new c_Frame)->m_new(t_i*m_width,0));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<160>");
	p_ApplyFlags(t_iflags);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<161>");
	return this;
}
c_Image* c_Image::p_Init2(gxtkSurface* t_surf,int t_x,int t_y,int t_iwidth,int t_iheight,int t_nframes,int t_iflags,c_Image* t_src,int t_srcx,int t_srcy,int t_srcw,int t_srch){
	DBG_ENTER("Image.Init")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_surf,"surf")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_iwidth,"iwidth")
	DBG_LOCAL(t_iheight,"iheight")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_LOCAL(t_src,"src")
	DBG_LOCAL(t_srcx,"srcx")
	DBG_LOCAL(t_srcy,"srcy")
	DBG_LOCAL(t_srcw,"srcw")
	DBG_LOCAL(t_srch,"srch")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<165>");
	gc_assign(m_surface,t_surf);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<166>");
	gc_assign(m_source,t_src);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<168>");
	m_width=t_iwidth;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<169>");
	m_height=t_iheight;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<171>");
	gc_assign(m_frames,Array<c_Frame* >(t_nframes));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<173>");
	int t_ix=t_x;
	int t_iy=t_y;
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<175>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<176>");
		if(t_ix+m_width>t_srcw){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<177>");
			t_ix=0;
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<178>");
			t_iy+=m_height;
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<180>");
		if(t_ix+m_width>t_srcw || t_iy+m_height>t_srch){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<181>");
			bbError(String(L"Image frame outside surface",27));
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<183>");
		gc_assign(m_frames.At(t_i),(new c_Frame)->m_new(t_ix+t_srcx,t_iy+t_srcy));
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<184>");
		t_ix+=m_width;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<187>");
	p_ApplyFlags(t_iflags);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<188>");
	return this;
}
int c_Image::p_Width(){
	DBG_ENTER("Image.Width")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<81>");
	return m_width;
}
int c_Image::p_Height(){
	DBG_ENTER("Image.Height")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<85>");
	return m_height;
}
int c_Image::p_Frames(){
	DBG_ENTER("Image.Frames")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<93>");
	int t_=m_frames.Length();
	return t_;
}
c_Image* c_Image::p_GrabImage(int t_x,int t_y,int t_width,int t_height,int t_nframes,int t_flags){
	DBG_ENTER("Image.GrabImage")
	c_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<109>");
	if(m_frames.Length()!=1){
		DBG_BLOCK();
		return 0;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<110>");
	c_Image* t_=((new c_Image)->m_new())->p_Init2(m_surface,t_x,t_y,t_width,t_height,t_nframes,t_flags,this,m_frames.At(0)->m_x,m_frames.At(0)->m_y,this->m_width,this->m_height);
	return t_;
}
void c_Image::mark(){
	Object::mark();
	gc_mark_q(m_surface);
	gc_mark_q(m_frames);
	gc_mark_q(m_source);
}
String c_Image::debug(){
	String t="(Image)\n";
	t+=dbg_decl("DefaultFlags",&c_Image::m_DefaultFlags);
	t+=dbg_decl("source",&m_source);
	t+=dbg_decl("surface",&m_surface);
	t+=dbg_decl("width",&m_width);
	t+=dbg_decl("height",&m_height);
	t+=dbg_decl("flags",&m_flags);
	t+=dbg_decl("frames",&m_frames);
	t+=dbg_decl("tx",&m_tx);
	t+=dbg_decl("ty",&m_ty);
	return t;
}
c_GraphicsContext::c_GraphicsContext(){
	m_defaultFont=0;
	m_font=0;
	m_firstChar=0;
	m_matrixSp=0;
	m_ix=FLOAT(1.0);
	m_iy=FLOAT(.0);
	m_jx=FLOAT(.0);
	m_jy=FLOAT(1.0);
	m_tx=FLOAT(.0);
	m_ty=FLOAT(.0);
	m_tformed=0;
	m_matDirty=0;
	m_color_r=FLOAT(.0);
	m_color_g=FLOAT(.0);
	m_color_b=FLOAT(.0);
	m_alpha=FLOAT(.0);
	m_blend=0;
	m_scissor_x=FLOAT(.0);
	m_scissor_y=FLOAT(.0);
	m_scissor_width=FLOAT(.0);
	m_scissor_height=FLOAT(.0);
	m_matrixStack=Array<Float >(192);
}
c_GraphicsContext* c_GraphicsContext::m_new(){
	DBG_ENTER("GraphicsContext.new")
	c_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<29>");
	return this;
}
int c_GraphicsContext::p_Validate(){
	DBG_ENTER("GraphicsContext.Validate")
	c_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<40>");
	if((m_matDirty)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<41>");
		bb_graphics_renderDevice->SetMatrix(bb_graphics_context->m_ix,bb_graphics_context->m_iy,bb_graphics_context->m_jx,bb_graphics_context->m_jy,bb_graphics_context->m_tx,bb_graphics_context->m_ty);
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<42>");
		m_matDirty=0;
	}
	return 0;
}
void c_GraphicsContext::mark(){
	Object::mark();
	gc_mark_q(m_defaultFont);
	gc_mark_q(m_font);
	gc_mark_q(m_matrixStack);
}
String c_GraphicsContext::debug(){
	String t="(GraphicsContext)\n";
	t+=dbg_decl("color_r",&m_color_r);
	t+=dbg_decl("color_g",&m_color_g);
	t+=dbg_decl("color_b",&m_color_b);
	t+=dbg_decl("alpha",&m_alpha);
	t+=dbg_decl("blend",&m_blend);
	t+=dbg_decl("ix",&m_ix);
	t+=dbg_decl("iy",&m_iy);
	t+=dbg_decl("jx",&m_jx);
	t+=dbg_decl("jy",&m_jy);
	t+=dbg_decl("tx",&m_tx);
	t+=dbg_decl("ty",&m_ty);
	t+=dbg_decl("tformed",&m_tformed);
	t+=dbg_decl("matDirty",&m_matDirty);
	t+=dbg_decl("scissor_x",&m_scissor_x);
	t+=dbg_decl("scissor_y",&m_scissor_y);
	t+=dbg_decl("scissor_width",&m_scissor_width);
	t+=dbg_decl("scissor_height",&m_scissor_height);
	t+=dbg_decl("matrixStack",&m_matrixStack);
	t+=dbg_decl("matrixSp",&m_matrixSp);
	t+=dbg_decl("font",&m_font);
	t+=dbg_decl("firstChar",&m_firstChar);
	t+=dbg_decl("defaultFont",&m_defaultFont);
	return t;
}
c_GraphicsContext* bb_graphics_context;
String bb_data_FixDataPath(String t_path){
	DBG_ENTER("FixDataPath")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/data.monkey<7>");
	int t_i=t_path.Find(String(L":/",2),0);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/data.monkey<8>");
	if(t_i!=-1 && t_path.Find(String(L"/",1),0)==t_i+1){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/data.monkey<9>");
	if(t_path.StartsWith(String(L"./",2)) || t_path.StartsWith(String(L"/",1))){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/data.monkey<10>");
	String t_=String(L"monkey://data/",14)+t_path;
	return t_;
}
c_Frame::c_Frame(){
	m_x=0;
	m_y=0;
}
c_Frame* c_Frame::m_new(int t_x,int t_y){
	DBG_ENTER("Frame.new")
	c_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<23>");
	this->m_x=t_x;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<24>");
	this->m_y=t_y;
	return this;
}
c_Frame* c_Frame::m_new2(){
	DBG_ENTER("Frame.new")
	c_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<18>");
	return this;
}
void c_Frame::mark(){
	Object::mark();
}
String c_Frame::debug(){
	String t="(Frame)\n";
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	return t;
}
c_Image* bb_graphics_LoadImage(String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("LoadImage")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<244>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<245>");
	if((t_surf)!=0){
		DBG_BLOCK();
		c_Image* t_=((new c_Image)->m_new())->p_Init(t_surf,t_frameCount,t_flags);
		return t_;
	}
	return 0;
}
c_Image* bb_graphics_LoadImage2(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
	DBG_ENTER("LoadImage")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<249>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<250>");
	if((t_surf)!=0){
		DBG_BLOCK();
		c_Image* t_=((new c_Image)->m_new())->p_Init2(t_surf,0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags,0,0,0,t_surf->Width(),t_surf->Height());
		return t_;
	}
	return 0;
}
int bb_graphics_SetFont(c_Image* t_font,int t_firstChar){
	DBG_ENTER("SetFont")
	DBG_LOCAL(t_font,"font")
	DBG_LOCAL(t_firstChar,"firstChar")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<551>");
	if(!((t_font)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<552>");
		if(!((bb_graphics_context->m_defaultFont)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<553>");
			gc_assign(bb_graphics_context->m_defaultFont,bb_graphics_LoadImage(String(L"mojo_font.png",13),96,2));
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<555>");
		t_font=bb_graphics_context->m_defaultFont;
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<556>");
		t_firstChar=32;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<558>");
	gc_assign(bb_graphics_context->m_font,t_font);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<559>");
	bb_graphics_context->m_firstChar=t_firstChar;
	return 0;
}
gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio* t_dev){
	DBG_ENTER("SetAudioDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/audio.monkey<22>");
	gc_assign(bb_audio_device,t_dev);
	return 0;
}
c_InputDevice::c_InputDevice(){
	m__joyStates=Array<c_JoyState* >(4);
	m__keyDown=Array<bool >(512);
	m__keyHitPut=0;
	m__keyHitQueue=Array<int >(33);
	m__keyHit=Array<int >(512);
	m__charGet=0;
	m__charPut=0;
	m__charQueue=Array<int >(32);
	m__mouseX=FLOAT(.0);
	m__mouseY=FLOAT(.0);
	m__touchX=Array<Float >(32);
	m__touchY=Array<Float >(32);
	m__accelX=FLOAT(.0);
	m__accelY=FLOAT(.0);
	m__accelZ=FLOAT(.0);
}
c_InputDevice* c_InputDevice::m_new(){
	DBG_ENTER("InputDevice.new")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<26>");
	for(int t_i=0;t_i<4;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<27>");
		gc_assign(m__joyStates.At(t_i),(new c_JoyState)->m_new());
	}
	return this;
}
void c_InputDevice::p_PutKeyHit(int t_key){
	DBG_ENTER("InputDevice.PutKeyHit")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<237>");
	if(m__keyHitPut==m__keyHitQueue.Length()){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<238>");
	m__keyHit.At(t_key)+=1;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<239>");
	m__keyHitQueue.At(m__keyHitPut)=t_key;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<240>");
	m__keyHitPut+=1;
}
void c_InputDevice::p_BeginUpdate(){
	DBG_ENTER("InputDevice.BeginUpdate")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<189>");
	for(int t_i=0;t_i<4;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<190>");
		c_JoyState* t_state=m__joyStates.At(t_i);
		DBG_LOCAL(t_state,"state")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<191>");
		if(!BBGame::Game()->PollJoystick(t_i,t_state->m_joyx,t_state->m_joyy,t_state->m_joyz,t_state->m_buttons)){
			DBG_BLOCK();
			break;
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<192>");
		for(int t_j=0;t_j<32;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<193>");
			int t_key=256+t_i*32+t_j;
			DBG_LOCAL(t_key,"key")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<194>");
			if(t_state->m_buttons.At(t_j)){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<195>");
				if(!m__keyDown.At(t_key)){
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<196>");
					m__keyDown.At(t_key)=true;
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<197>");
					p_PutKeyHit(t_key);
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<200>");
				m__keyDown.At(t_key)=false;
			}
		}
	}
}
void c_InputDevice::p_EndUpdate(){
	DBG_ENTER("InputDevice.EndUpdate")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<207>");
	for(int t_i=0;t_i<m__keyHitPut;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<208>");
		m__keyHit.At(m__keyHitQueue.At(t_i))=0;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<210>");
	m__keyHitPut=0;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<211>");
	m__charGet=0;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<212>");
	m__charPut=0;
}
void c_InputDevice::p_KeyEvent(int t_event,int t_data){
	DBG_ENTER("InputDevice.KeyEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<111>");
	int t_1=t_event;
	DBG_LOCAL(t_1,"1")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<112>");
	if(t_1==1){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<113>");
		if(!m__keyDown.At(t_data)){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<114>");
			m__keyDown.At(t_data)=true;
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<115>");
			p_PutKeyHit(t_data);
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<116>");
			if(t_data==1){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<117>");
				m__keyDown.At(384)=true;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<118>");
				p_PutKeyHit(384);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<119>");
				if(t_data==384){
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<120>");
					m__keyDown.At(1)=true;
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<121>");
					p_PutKeyHit(1);
				}
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<124>");
		if(t_1==2){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<125>");
			if(m__keyDown.At(t_data)){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<126>");
				m__keyDown.At(t_data)=false;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<127>");
				if(t_data==1){
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<128>");
					m__keyDown.At(384)=false;
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<129>");
					if(t_data==384){
						DBG_BLOCK();
						DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<130>");
						m__keyDown.At(1)=false;
					}
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<133>");
			if(t_1==3){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<134>");
				if(m__charPut<m__charQueue.Length()){
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<135>");
					m__charQueue.At(m__charPut)=t_data;
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<136>");
					m__charPut+=1;
				}
			}
		}
	}
}
void c_InputDevice::p_MouseEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("InputDevice.MouseEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<142>");
	int t_2=t_event;
	DBG_LOCAL(t_2,"2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<143>");
	if(t_2==4){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<144>");
		p_KeyEvent(1,1+t_data);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<145>");
		if(t_2==5){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<146>");
			p_KeyEvent(2,1+t_data);
			return;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<148>");
			if(t_2==6){
				DBG_BLOCK();
			}else{
				DBG_BLOCK();
				return;
			}
		}
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<152>");
	m__mouseX=t_x;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<153>");
	m__mouseY=t_y;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<154>");
	m__touchX.At(0)=t_x;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<155>");
	m__touchY.At(0)=t_y;
}
void c_InputDevice::p_TouchEvent(int t_event,int t_data,Float t_x,Float t_y){
	DBG_ENTER("InputDevice.TouchEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<159>");
	int t_3=t_event;
	DBG_LOCAL(t_3,"3")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<160>");
	if(t_3==7){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<161>");
		p_KeyEvent(1,384+t_data);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<162>");
		if(t_3==8){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<163>");
			p_KeyEvent(2,384+t_data);
			return;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<165>");
			if(t_3==9){
				DBG_BLOCK();
			}else{
				DBG_BLOCK();
				return;
			}
		}
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<169>");
	m__touchX.At(t_data)=t_x;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<170>");
	m__touchY.At(t_data)=t_y;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<171>");
	if(t_data==0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<172>");
		m__mouseX=t_x;
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<173>");
		m__mouseY=t_y;
	}
}
void c_InputDevice::p_MotionEvent(int t_event,int t_data,Float t_x,Float t_y,Float t_z){
	DBG_ENTER("InputDevice.MotionEvent")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_event,"event")
	DBG_LOCAL(t_data,"data")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_z,"z")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<178>");
	int t_4=t_event;
	DBG_LOCAL(t_4,"4")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<179>");
	if(t_4==10){
		DBG_BLOCK();
	}else{
		DBG_BLOCK();
		return;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<183>");
	m__accelX=t_x;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<184>");
	m__accelY=t_y;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<185>");
	m__accelZ=t_z;
}
int c_InputDevice::p_KeyHit(int t_key){
	DBG_ENTER("InputDevice.KeyHit")
	c_InputDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<52>");
	if(t_key>0 && t_key<512){
		DBG_BLOCK();
		return m__keyHit.At(t_key);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<53>");
	return 0;
}
void c_InputDevice::mark(){
	Object::mark();
	gc_mark_q(m__joyStates);
	gc_mark_q(m__keyDown);
	gc_mark_q(m__keyHitQueue);
	gc_mark_q(m__keyHit);
	gc_mark_q(m__charQueue);
	gc_mark_q(m__touchX);
	gc_mark_q(m__touchY);
}
String c_InputDevice::debug(){
	String t="(InputDevice)\n";
	t+=dbg_decl("_keyDown",&m__keyDown);
	t+=dbg_decl("_keyHit",&m__keyHit);
	t+=dbg_decl("_keyHitQueue",&m__keyHitQueue);
	t+=dbg_decl("_keyHitPut",&m__keyHitPut);
	t+=dbg_decl("_charQueue",&m__charQueue);
	t+=dbg_decl("_charPut",&m__charPut);
	t+=dbg_decl("_charGet",&m__charGet);
	t+=dbg_decl("_mouseX",&m__mouseX);
	t+=dbg_decl("_mouseY",&m__mouseY);
	t+=dbg_decl("_touchX",&m__touchX);
	t+=dbg_decl("_touchY",&m__touchY);
	t+=dbg_decl("_accelX",&m__accelX);
	t+=dbg_decl("_accelY",&m__accelY);
	t+=dbg_decl("_accelZ",&m__accelZ);
	t+=dbg_decl("_joyStates",&m__joyStates);
	return t;
}
c_JoyState::c_JoyState(){
	m_joyx=Array<Float >(2);
	m_joyy=Array<Float >(2);
	m_joyz=Array<Float >(2);
	m_buttons=Array<bool >(32);
}
c_JoyState* c_JoyState::m_new(){
	DBG_ENTER("JoyState.new")
	c_JoyState *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/inputdevice.monkey<14>");
	return this;
}
void c_JoyState::mark(){
	Object::mark();
	gc_mark_q(m_joyx);
	gc_mark_q(m_joyy);
	gc_mark_q(m_joyz);
	gc_mark_q(m_buttons);
}
String c_JoyState::debug(){
	String t="(JoyState)\n";
	t+=dbg_decl("joyx",&m_joyx);
	t+=dbg_decl("joyy",&m_joyy);
	t+=dbg_decl("joyz",&m_joyz);
	t+=dbg_decl("buttons",&m_buttons);
	return t;
}
c_InputDevice* bb_input_device;
int bb_input_SetInputDevice(c_InputDevice* t_dev){
	DBG_ENTER("SetInputDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/input.monkey<22>");
	gc_assign(bb_input_device,t_dev);
	return 0;
}
int bb_app__devWidth;
int bb_app__devHeight;
void bb_app_ValidateDeviceWindow(bool t_notifyApp){
	DBG_ENTER("ValidateDeviceWindow")
	DBG_LOCAL(t_notifyApp,"notifyApp")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<57>");
	int t_w=bb_app__game->GetDeviceWidth();
	DBG_LOCAL(t_w,"w")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<58>");
	int t_h=bb_app__game->GetDeviceHeight();
	DBG_LOCAL(t_h,"h")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<59>");
	if(t_w==bb_app__devWidth && t_h==bb_app__devHeight){
		DBG_BLOCK();
		return;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<60>");
	bb_app__devWidth=t_w;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<61>");
	bb_app__devHeight=t_h;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<62>");
	if(t_notifyApp){
		DBG_BLOCK();
		bb_app__app->p_OnResize();
	}
}
c_DisplayMode::c_DisplayMode(){
	m__width=0;
	m__height=0;
}
c_DisplayMode* c_DisplayMode::m_new(int t_width,int t_height){
	DBG_ENTER("DisplayMode.new")
	c_DisplayMode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<192>");
	m__width=t_width;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<193>");
	m__height=t_height;
	return this;
}
c_DisplayMode* c_DisplayMode::m_new2(){
	DBG_ENTER("DisplayMode.new")
	c_DisplayMode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<189>");
	return this;
}
void c_DisplayMode::mark(){
	Object::mark();
}
String c_DisplayMode::debug(){
	String t="(DisplayMode)\n";
	t+=dbg_decl("_width",&m__width);
	t+=dbg_decl("_height",&m__height);
	return t;
}
c_Map::c_Map(){
	m_root=0;
}
c_Map* c_Map::m_new(){
	DBG_ENTER("Map.new")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<7>");
	return this;
}
c_Node* c_Map::p_FindNode(int t_key){
	DBG_ENTER("Map.FindNode")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<157>");
	c_Node* t_node=m_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<159>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<160>");
		int t_cmp=p_Compare(t_key,t_node->m_key);
		DBG_LOCAL(t_cmp,"cmp")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<161>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<162>");
			t_node=t_node->m_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<163>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<164>");
				t_node=t_node->m_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<166>");
				return t_node;
			}
		}
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<169>");
	return t_node;
}
bool c_Map::p_Contains(int t_key){
	DBG_ENTER("Map.Contains")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<25>");
	bool t_=p_FindNode(t_key)!=0;
	return t_;
}
int c_Map::p_RotateLeft(c_Node* t_node){
	DBG_ENTER("Map.RotateLeft")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<251>");
	c_Node* t_child=t_node->m_right;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<252>");
	gc_assign(t_node->m_right,t_child->m_left);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<253>");
	if((t_child->m_left)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<254>");
		gc_assign(t_child->m_left->m_parent,t_node);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<256>");
	gc_assign(t_child->m_parent,t_node->m_parent);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<257>");
	if((t_node->m_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<258>");
		if(t_node==t_node->m_parent->m_left){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<259>");
			gc_assign(t_node->m_parent->m_left,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<261>");
			gc_assign(t_node->m_parent->m_right,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<264>");
		gc_assign(m_root,t_child);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<266>");
	gc_assign(t_child->m_left,t_node);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<267>");
	gc_assign(t_node->m_parent,t_child);
	return 0;
}
int c_Map::p_RotateRight(c_Node* t_node){
	DBG_ENTER("Map.RotateRight")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<271>");
	c_Node* t_child=t_node->m_left;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<272>");
	gc_assign(t_node->m_left,t_child->m_right);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<273>");
	if((t_child->m_right)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<274>");
		gc_assign(t_child->m_right->m_parent,t_node);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<276>");
	gc_assign(t_child->m_parent,t_node->m_parent);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<277>");
	if((t_node->m_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<278>");
		if(t_node==t_node->m_parent->m_right){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<279>");
			gc_assign(t_node->m_parent->m_right,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<281>");
			gc_assign(t_node->m_parent->m_left,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<284>");
		gc_assign(m_root,t_child);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<286>");
	gc_assign(t_child->m_right,t_node);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<287>");
	gc_assign(t_node->m_parent,t_child);
	return 0;
}
int c_Map::p_InsertFixup(c_Node* t_node){
	DBG_ENTER("Map.InsertFixup")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<212>");
	while(((t_node->m_parent)!=0) && t_node->m_parent->m_color==-1 && ((t_node->m_parent->m_parent)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<213>");
		if(t_node->m_parent==t_node->m_parent->m_parent->m_left){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<214>");
			c_Node* t_uncle=t_node->m_parent->m_parent->m_right;
			DBG_LOCAL(t_uncle,"uncle")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<215>");
			if(((t_uncle)!=0) && t_uncle->m_color==-1){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<216>");
				t_node->m_parent->m_color=1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<217>");
				t_uncle->m_color=1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<218>");
				t_uncle->m_parent->m_color=-1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<219>");
				t_node=t_uncle->m_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<221>");
				if(t_node==t_node->m_parent->m_right){
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<222>");
					t_node=t_node->m_parent;
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<223>");
					p_RotateLeft(t_node);
				}
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<225>");
				t_node->m_parent->m_color=1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<226>");
				t_node->m_parent->m_parent->m_color=-1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<227>");
				p_RotateRight(t_node->m_parent->m_parent);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<230>");
			c_Node* t_uncle2=t_node->m_parent->m_parent->m_left;
			DBG_LOCAL(t_uncle2,"uncle")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<231>");
			if(((t_uncle2)!=0) && t_uncle2->m_color==-1){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<232>");
				t_node->m_parent->m_color=1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<233>");
				t_uncle2->m_color=1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<234>");
				t_uncle2->m_parent->m_color=-1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<235>");
				t_node=t_uncle2->m_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<237>");
				if(t_node==t_node->m_parent->m_left){
					DBG_BLOCK();
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<238>");
					t_node=t_node->m_parent;
					DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<239>");
					p_RotateRight(t_node);
				}
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<241>");
				t_node->m_parent->m_color=1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<242>");
				t_node->m_parent->m_parent->m_color=-1;
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<243>");
				p_RotateLeft(t_node->m_parent->m_parent);
			}
		}
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<247>");
	m_root->m_color=1;
	return 0;
}
bool c_Map::p_Set(int t_key,c_DisplayMode* t_value){
	DBG_ENTER("Map.Set")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<29>");
	c_Node* t_node=m_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<30>");
	c_Node* t_parent=0;
	int t_cmp=0;
	DBG_LOCAL(t_parent,"parent")
	DBG_LOCAL(t_cmp,"cmp")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<32>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<33>");
		t_parent=t_node;
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<34>");
		t_cmp=p_Compare(t_key,t_node->m_key);
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<35>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<36>");
			t_node=t_node->m_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<37>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<38>");
				t_node=t_node->m_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<40>");
				gc_assign(t_node->m_value,t_value);
				DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<41>");
				return false;
			}
		}
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<45>");
	t_node=(new c_Node)->m_new(t_key,t_value,-1,t_parent);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<47>");
	if((t_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<48>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<49>");
			gc_assign(t_parent->m_right,t_node);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<51>");
			gc_assign(t_parent->m_left,t_node);
		}
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<53>");
		p_InsertFixup(t_node);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<55>");
		gc_assign(m_root,t_node);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<57>");
	return true;
}
bool c_Map::p_Insert(int t_key,c_DisplayMode* t_value){
	DBG_ENTER("Map.Insert")
	c_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<126>");
	bool t_=p_Set(t_key,t_value);
	return t_;
}
void c_Map::mark(){
	Object::mark();
	gc_mark_q(m_root);
}
String c_Map::debug(){
	String t="(Map)\n";
	t+=dbg_decl("root",&m_root);
	return t;
}
c_IntMap::c_IntMap(){
}
c_IntMap* c_IntMap::m_new(){
	DBG_ENTER("IntMap.new")
	c_IntMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<534>");
	c_Map::m_new();
	return this;
}
int c_IntMap::p_Compare(int t_lhs,int t_rhs){
	DBG_ENTER("IntMap.Compare")
	c_IntMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_lhs,"lhs")
	DBG_LOCAL(t_rhs,"rhs")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<537>");
	int t_=t_lhs-t_rhs;
	return t_;
}
void c_IntMap::mark(){
	c_Map::mark();
}
String c_IntMap::debug(){
	String t="(IntMap)\n";
	t=c_Map::debug()+t;
	return t;
}
c_Stack::c_Stack(){
	m_data=Array<c_DisplayMode* >();
	m_length=0;
}
c_Stack* c_Stack::m_new(){
	DBG_ENTER("Stack.new")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
c_Stack* c_Stack::m_new2(Array<c_DisplayMode* > t_data){
	DBG_ENTER("Stack.new")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<13>");
	gc_assign(this->m_data,t_data.Slice(0));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<14>");
	this->m_length=t_data.Length();
	return this;
}
void c_Stack::p_Push(c_DisplayMode* t_value){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<67>");
	if(m_length==m_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<68>");
		gc_assign(m_data,m_data.Resize(m_length*2+10));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<70>");
	gc_assign(m_data.At(m_length),t_value);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<71>");
	m_length+=1;
}
void c_Stack::p_Push2(Array<c_DisplayMode* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<79>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<80>");
		p_Push(t_values.At(t_offset+t_i));
	}
}
void c_Stack::p_Push3(Array<c_DisplayMode* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<75>");
	p_Push2(t_values,t_offset,t_values.Length()-t_offset);
}
Array<c_DisplayMode* > c_Stack::p_ToArray(){
	DBG_ENTER("Stack.ToArray")
	c_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<18>");
	Array<c_DisplayMode* > t_t=Array<c_DisplayMode* >(m_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<m_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),m_data.At(t_i));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/stack.monkey<22>");
	return t_t;
}
void c_Stack::mark(){
	Object::mark();
	gc_mark_q(m_data);
}
String c_Stack::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&m_data);
	t+=dbg_decl("length",&m_length);
	return t;
}
c_Node::c_Node(){
	m_key=0;
	m_right=0;
	m_left=0;
	m_value=0;
	m_color=0;
	m_parent=0;
}
c_Node* c_Node::m_new(int t_key,c_DisplayMode* t_value,int t_color,c_Node* t_parent){
	DBG_ENTER("Node.new")
	c_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_LOCAL(t_color,"color")
	DBG_LOCAL(t_parent,"parent")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<364>");
	this->m_key=t_key;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<365>");
	gc_assign(this->m_value,t_value);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<366>");
	this->m_color=t_color;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<367>");
	gc_assign(this->m_parent,t_parent);
	return this;
}
c_Node* c_Node::m_new2(){
	DBG_ENTER("Node.new")
	c_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/map.monkey<361>");
	return this;
}
void c_Node::mark(){
	Object::mark();
	gc_mark_q(m_right);
	gc_mark_q(m_left);
	gc_mark_q(m_value);
	gc_mark_q(m_parent);
}
String c_Node::debug(){
	String t="(Node)\n";
	t+=dbg_decl("key",&m_key);
	t+=dbg_decl("value",&m_value);
	t+=dbg_decl("color",&m_color);
	t+=dbg_decl("parent",&m_parent);
	t+=dbg_decl("left",&m_left);
	t+=dbg_decl("right",&m_right);
	return t;
}
Array<c_DisplayMode* > bb_app__displayModes;
c_DisplayMode* bb_app__desktopMode;
int bb_app_DeviceWidth(){
	DBG_ENTER("DeviceWidth")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<263>");
	return bb_app__devWidth;
}
int bb_app_DeviceHeight(){
	DBG_ENTER("DeviceHeight")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<267>");
	return bb_app__devHeight;
}
void bb_app_EnumDisplayModes(){
	DBG_ENTER("EnumDisplayModes")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<33>");
	Array<BBDisplayMode* > t_modes=bb_app__game->GetDisplayModes();
	DBG_LOCAL(t_modes,"modes")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<34>");
	c_IntMap* t_mmap=(new c_IntMap)->m_new();
	DBG_LOCAL(t_mmap,"mmap")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<35>");
	c_Stack* t_mstack=(new c_Stack)->m_new();
	DBG_LOCAL(t_mstack,"mstack")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<36>");
	for(int t_i=0;t_i<t_modes.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<37>");
		int t_w=t_modes.At(t_i)->width;
		DBG_LOCAL(t_w,"w")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<38>");
		int t_h=t_modes.At(t_i)->height;
		DBG_LOCAL(t_h,"h")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<39>");
		int t_size=t_w<<16|t_h;
		DBG_LOCAL(t_size,"size")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<40>");
		if(t_mmap->p_Contains(t_size)){
			DBG_BLOCK();
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<42>");
			c_DisplayMode* t_mode=(new c_DisplayMode)->m_new(t_modes.At(t_i)->width,t_modes.At(t_i)->height);
			DBG_LOCAL(t_mode,"mode")
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<43>");
			t_mmap->p_Insert(t_size,t_mode);
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<44>");
			t_mstack->p_Push(t_mode);
		}
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<47>");
	gc_assign(bb_app__displayModes,t_mstack->p_ToArray());
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<48>");
	BBDisplayMode* t_mode2=bb_app__game->GetDesktopMode();
	DBG_LOCAL(t_mode2,"mode")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<49>");
	if((t_mode2)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<50>");
		gc_assign(bb_app__desktopMode,(new c_DisplayMode)->m_new(t_mode2->width,t_mode2->height));
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<52>");
		gc_assign(bb_app__desktopMode,(new c_DisplayMode)->m_new(bb_app_DeviceWidth(),bb_app_DeviceHeight()));
	}
}
gxtkGraphics* bb_graphics_renderDevice;
int bb_graphics_SetMatrix(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<317>");
	bb_graphics_context->m_ix=t_ix;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<318>");
	bb_graphics_context->m_iy=t_iy;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<319>");
	bb_graphics_context->m_jx=t_jx;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<320>");
	bb_graphics_context->m_jy=t_jy;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<321>");
	bb_graphics_context->m_tx=t_tx;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<322>");
	bb_graphics_context->m_ty=t_ty;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<323>");
	bb_graphics_context->m_tformed=((t_ix!=FLOAT(1.0) || t_iy!=FLOAT(0.0) || t_jx!=FLOAT(0.0) || t_jy!=FLOAT(1.0) || t_tx!=FLOAT(0.0) || t_ty!=FLOAT(0.0))?1:0);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<324>");
	bb_graphics_context->m_matDirty=1;
	return 0;
}
int bb_graphics_SetMatrix2(Array<Float > t_m){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<313>");
	bb_graphics_SetMatrix(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_SetColor(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("SetColor")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<259>");
	bb_graphics_context->m_color_r=t_r;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<260>");
	bb_graphics_context->m_color_g=t_g;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<261>");
	bb_graphics_context->m_color_b=t_b;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<262>");
	bb_graphics_renderDevice->SetColor(t_r,t_g,t_b);
	return 0;
}
int bb_graphics_SetAlpha(Float t_alpha){
	DBG_ENTER("SetAlpha")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<276>");
	bb_graphics_context->m_alpha=t_alpha;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<277>");
	bb_graphics_renderDevice->SetAlpha(t_alpha);
	return 0;
}
int bb_graphics_SetBlend(int t_blend){
	DBG_ENTER("SetBlend")
	DBG_LOCAL(t_blend,"blend")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<285>");
	bb_graphics_context->m_blend=t_blend;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<286>");
	bb_graphics_renderDevice->SetBlend(t_blend);
	return 0;
}
int bb_graphics_SetScissor(Float t_x,Float t_y,Float t_width,Float t_height){
	DBG_ENTER("SetScissor")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<294>");
	bb_graphics_context->m_scissor_x=t_x;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<295>");
	bb_graphics_context->m_scissor_y=t_y;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<296>");
	bb_graphics_context->m_scissor_width=t_width;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<297>");
	bb_graphics_context->m_scissor_height=t_height;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<298>");
	bb_graphics_renderDevice->SetScissor(int(t_x),int(t_y),int(t_width),int(t_height));
	return 0;
}
int bb_graphics_BeginRender(){
	DBG_ENTER("BeginRender")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<220>");
	gc_assign(bb_graphics_renderDevice,bb_graphics_device);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<221>");
	bb_graphics_context->m_matrixSp=0;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<222>");
	bb_graphics_SetMatrix(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<223>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<224>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<225>");
	bb_graphics_SetBlend(0);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<226>");
	bb_graphics_SetScissor(FLOAT(0.0),FLOAT(0.0),Float(bb_app_DeviceWidth()),Float(bb_app_DeviceHeight()));
	return 0;
}
int bb_graphics_EndRender(){
	DBG_ENTER("EndRender")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<230>");
	bb_graphics_renderDevice=0;
	return 0;
}
c_BBGameEvent::c_BBGameEvent(){
}
void c_BBGameEvent::mark(){
	Object::mark();
}
String c_BBGameEvent::debug(){
	String t="(BBGameEvent)\n";
	return t;
}
void bb_app_EndApp(){
	DBG_ENTER("EndApp")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<259>");
	bbError(String());
}
c_RoomMap::c_RoomMap(){
	m_MapWidth=6;
	m_MapHeight=6;
	m_Map=Array<Array<c_Room* > >(6);
	m_currentRoom=0;
	m_roomNum=15;
	m_MapPSize=32;
	m_MapXOffset=480;
}
c_RoomMap* c_RoomMap::m_new(){
	DBG_ENTER("RoomMap.new")
	c_RoomMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<234>");
	return this;
}
int c_RoomMap::p_Build(){
	DBG_ENTER("RoomMap.Build")
	c_RoomMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<244>");
	int t_RoomCount=0;
	DBG_LOCAL(t_RoomCount,"RoomCount")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<246>");
	for(int t_i=0;t_i<=m_MapWidth-1;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<247>");
		gc_assign(m_Map.At(t_i),Array<c_Room* >(m_MapHeight));
	}
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<251>");
	for(int t_i2=0;t_i2<=m_MapWidth-1;t_i2=t_i2+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i2,"i")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<252>");
		for(int t_j=0;t_j<=m_MapHeight-1;t_j=t_j+1){
			DBG_BLOCK();
			DBG_LOCAL(t_j,"j")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<253>");
			gc_assign(m_Map.At(t_i2).At(t_j),(new c_Room)->m_new(0,t_i2,t_j));
		}
	}
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<259>");
	int t_xr=int(bb_random_Rnd()*Float(m_MapWidth));
	DBG_LOCAL(t_xr,"xr")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<260>");
	int t_yr=int(bb_random_Rnd()*Float(m_MapHeight));
	DBG_LOCAL(t_yr,"yr")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<261>");
	m_Map.At(t_xr).At(t_yr)->p_UpdateType(1);
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<262>");
	gc_assign(m_currentRoom,m_Map.At(t_xr).At(t_yr));
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<263>");
	t_RoomCount+=1;
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<265>");
	if(t_xr>0 && t_xr<m_MapWidth-1){
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<266>");
		m_Map.At(t_xr+1).At(t_yr)->p_UpdateNeighbours(1);
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<267>");
		m_Map.At(t_xr-1).At(t_yr)->p_UpdateNeighbours(1);
	}else{
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<268>");
		if(t_xr==0){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<269>");
			m_Map.At(t_xr+1).At(t_yr)->p_UpdateNeighbours(1);
		}else{
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<270>");
			if(t_xr==m_MapWidth-1){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<271>");
				m_Map.At(t_xr-1).At(t_yr)->p_UpdateNeighbours(1);
			}
		}
	}
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<274>");
	if(t_yr>0 && t_yr<m_MapHeight-1){
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<275>");
		m_Map.At(t_xr).At(t_yr+1)->p_UpdateNeighbours(1);
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<276>");
		m_Map.At(t_xr).At(t_yr-1)->p_UpdateNeighbours(1);
	}else{
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<277>");
		if(t_yr==0){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<278>");
			m_Map.At(t_xr).At(t_yr+1)->p_UpdateNeighbours(1);
		}else{
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<279>");
			if(t_yr==m_MapHeight-1){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<280>");
				m_Map.At(t_xr).At(t_yr-1)->p_UpdateNeighbours(1);
			}
		}
	}
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<284>");
	while(t_RoomCount<m_roomNum){
		DBG_BLOCK();
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<285>");
		c_List* t_Valid=(new c_List)->m_new();
		DBG_LOCAL(t_Valid,"Valid")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<287>");
		for(int t_i3=0;t_i3<=m_MapWidth-1;t_i3=t_i3+1){
			DBG_BLOCK();
			DBG_LOCAL(t_i3,"i")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<288>");
			for(int t_j2=0;t_j2<=m_MapHeight-1;t_j2=t_j2+1){
				DBG_BLOCK();
				DBG_LOCAL(t_j2,"j")
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<289>");
				if(m_Map.At(t_i3).At(t_j2)->p_GetNeighbours()==1 && m_Map.At(t_i3).At(t_j2)->p_GetType()==0){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<290>");
					t_Valid->p_AddLast(m_Map.At(t_i3).At(t_j2));
				}
			}
		}
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<296>");
		Array<c_Room* > t_validRoomArray=t_Valid->p_ToArray();
		DBG_LOCAL(t_validRoomArray,"validRoomArray")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<299>");
		int t_ir=int(bb_random_Rnd()*Float(t_validRoomArray.Length())-FLOAT(1.0));
		DBG_LOCAL(t_ir,"ir")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<300>");
		t_validRoomArray.At(t_ir)->p_UpdateType(2);
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<301>");
		t_RoomCount+=1;
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<304>");
		int t_x=t_validRoomArray.At(t_ir)->p_GetX();
		DBG_LOCAL(t_x,"x")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<305>");
		int t_y=t_validRoomArray.At(t_ir)->p_GetY();
		DBG_LOCAL(t_y,"y")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<308>");
		if(t_x>0 && t_x<m_MapWidth-1){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<309>");
			m_Map.At(t_x+1).At(t_y)->p_UpdateNeighbours(1);
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<310>");
			m_Map.At(t_x-1).At(t_y)->p_UpdateNeighbours(1);
		}else{
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<311>");
			if(t_x==0){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<312>");
				m_Map.At(t_x+1).At(t_y)->p_UpdateNeighbours(1);
			}else{
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<313>");
				if(t_x==m_MapWidth-1){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<314>");
					m_Map.At(t_x-1).At(t_y)->p_UpdateNeighbours(1);
				}
			}
		}
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<317>");
		if(t_y>0 && t_y<m_MapHeight-1){
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<318>");
			m_Map.At(t_x).At(t_y+1)->p_UpdateNeighbours(1);
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<319>");
			m_Map.At(t_x).At(t_y-1)->p_UpdateNeighbours(1);
		}else{
			DBG_BLOCK();
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<320>");
			if(t_y==0){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<321>");
				m_Map.At(t_x).At(t_y+1)->p_UpdateNeighbours(1);
			}else{
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<322>");
				if(t_y==m_MapHeight-1){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<323>");
					m_Map.At(t_x).At(t_y-1)->p_UpdateNeighbours(1);
				}
			}
		}
	}
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<329>");
	for(int t_x2=0;t_x2<=m_MapWidth-1;t_x2=t_x2+1){
		DBG_BLOCK();
		DBG_LOCAL(t_x2,"x")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<330>");
		for(int t_y2=0;t_y2<=m_MapHeight-1;t_y2=t_y2+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y2,"y")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<331>");
			if(m_Map.At(t_x2).At(t_y2)->p_GetType()!=0){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<332>");
				if(t_y2>0){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<333>");
					if(m_Map.At(t_x2).At(t_y2-1)->p_GetType()!=0 && t_y2>0){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<334>");
						m_Map.At(t_x2).At(t_y2)->p_SetnDoor(String(L"1",1));
					}
				}
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<337>");
				if(t_y2<m_MapHeight-1){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<338>");
					if(m_Map.At(t_x2).At(t_y2+1)->p_GetType()!=0 && t_y2<m_MapHeight-1){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<339>");
						m_Map.At(t_x2).At(t_y2)->p_SetsDoor(String(L"1",1));
					}
				}
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<342>");
				if(t_x2>0){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<343>");
					if(m_Map.At(t_x2-1).At(t_y2)->p_GetType()!=0 && t_x2>0){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<344>");
						m_Map.At(t_x2).At(t_y2)->p_SetwDoor(String(L"1",1));
					}
				}
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<347>");
				if(t_x2<m_MapWidth-1){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<348>");
					if(m_Map.At(t_x2+1).At(t_y2)->p_GetType()!=0 && t_x2<m_MapWidth-1){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<349>");
						m_Map.At(t_x2).At(t_y2)->p_SeteDoor(String(L"1",1));
					}
				}
			}
		}
	}
	return 0;
}
int c_RoomMap::p_Reset(){
	DBG_ENTER("RoomMap.Reset")
	c_RoomMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<455>");
	for(int t_x=0;t_x<=m_MapWidth-1;t_x=t_x+1){
		DBG_BLOCK();
		DBG_LOCAL(t_x,"x")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<456>");
		for(int t_y=0;t_y<=m_MapHeight-1;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<457>");
			m_Map.At(t_x).At(t_y)->p_Reset();
		}
	}
	return 0;
}
int c_RoomMap::p_DrawRoomFloor(){
	DBG_ENTER("RoomMap.DrawRoomFloor")
	c_RoomMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<359>");
	m_currentRoom->p_DrawFloor();
	return 0;
}
int c_RoomMap::p_DrawRoomWalls(){
	DBG_ENTER("RoomMap.DrawRoomWalls")
	c_RoomMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<363>");
	m_currentRoom->p_DrawWalls();
	return 0;
}
int c_RoomMap::p_DrawMap(){
	DBG_ENTER("RoomMap.DrawMap")
	c_RoomMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<371>");
	for(int t_x=0;t_x<=m_MapWidth-1;t_x=t_x+1){
		DBG_BLOCK();
		DBG_LOCAL(t_x,"x")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<372>");
		for(int t_y=0;t_y<=m_MapHeight-1;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<373>");
			int t_4=m_Map.At(t_x).At(t_y)->p_GetType();
			DBG_LOCAL(t_4,"4")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<374>");
			if(t_4==0){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<375>");
				bb_graphics_SetColor(FLOAT(0.0),FLOAT(0.0),FLOAT(0.0));
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<376>");
				bb_graphics_DrawRect(Float(t_x*m_MapPSize+m_MapXOffset),Float(t_y*m_MapPSize),Float(m_MapPSize),Float(m_MapPSize));
			}else{
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<377>");
				if(t_4==1){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<378>");
					bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<379>");
					bb_graphics_DrawRect(Float(t_x*m_MapPSize+m_MapXOffset),Float(t_y*m_MapPSize),Float(m_MapPSize),Float(m_MapPSize));
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<380>");
					String t_5=m_Map.At(t_x).At(t_y)->p_GetDoors();
					DBG_LOCAL(t_5,"5")
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<381>");
					if(t_5==String(L"1000",4)){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<382>");
						bb_LayeredLevel_DrawRoom(String(L"StartRoom/N.png",15),t_x,t_y,m_MapPSize,m_MapXOffset);
					}else{
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<383>");
						if(t_5==String(L"0100",4)){
							DBG_BLOCK();
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<384>");
							bb_LayeredLevel_DrawRoom(String(L"StartRoom/E.png",15),t_x,t_y,m_MapPSize,m_MapXOffset);
						}else{
							DBG_BLOCK();
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<385>");
							if(t_5==String(L"0010",4)){
								DBG_BLOCK();
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<386>");
								bb_LayeredLevel_DrawRoom(String(L"StartRoom/S.png",15),t_x,t_y,m_MapPSize,m_MapXOffset);
							}else{
								DBG_BLOCK();
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<387>");
								if(t_5==String(L"0001",4)){
									DBG_BLOCK();
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<388>");
									bb_LayeredLevel_DrawRoom(String(L"StartRoom/W.png",15),t_x,t_y,m_MapPSize,m_MapXOffset);
								}else{
									DBG_BLOCK();
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<389>");
									if(t_5==String(L"1100",4)){
										DBG_BLOCK();
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<390>");
										bb_LayeredLevel_DrawRoom(String(L"StartRoom/NE.png",16),t_x,t_y,m_MapPSize,m_MapXOffset);
									}else{
										DBG_BLOCK();
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<391>");
										if(t_5==String(L"1010",4)){
											DBG_BLOCK();
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<392>");
											bb_LayeredLevel_DrawRoom(String(L"StartRoom/NS.png",16),t_x,t_y,m_MapPSize,m_MapXOffset);
										}else{
											DBG_BLOCK();
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<393>");
											if(t_5==String(L"1001",4)){
												DBG_BLOCK();
												DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<394>");
												bb_LayeredLevel_DrawRoom(String(L"StartRoom/NW.png",16),t_x,t_y,m_MapPSize,m_MapXOffset);
											}else{
												DBG_BLOCK();
												DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<395>");
												if(t_5==String(L"1110",4)){
													DBG_BLOCK();
													DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<396>");
													bb_LayeredLevel_DrawRoom(String(L"StartRoom/NES.png",17),t_x,t_y,m_MapPSize,m_MapXOffset);
												}else{
													DBG_BLOCK();
													DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<397>");
													if(t_5==String(L"1101",4)){
														DBG_BLOCK();
														DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<398>");
														bb_LayeredLevel_DrawRoom(String(L"StartRoom/NEW.png",17),t_x,t_y,m_MapPSize,m_MapXOffset);
													}else{
														DBG_BLOCK();
														DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<399>");
														if(t_5==String(L"0110",4)){
															DBG_BLOCK();
															DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<400>");
															bb_LayeredLevel_DrawRoom(String(L"StartRoom/ES.png",16),t_x,t_y,m_MapPSize,m_MapXOffset);
														}else{
															DBG_BLOCK();
															DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<401>");
															if(t_5==String(L"0011",4)){
																DBG_BLOCK();
																DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<402>");
																bb_LayeredLevel_DrawRoom(String(L"StartRoom/SW.png",16),t_x,t_y,m_MapPSize,m_MapXOffset);
															}else{
																DBG_BLOCK();
																DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<403>");
																if(t_5==String(L"0111",4)){
																	DBG_BLOCK();
																	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<404>");
																	bb_LayeredLevel_DrawRoom(String(L"StartRoom/ESW.png",17),t_x,t_y,m_MapPSize,m_MapXOffset);
																}else{
																	DBG_BLOCK();
																	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<405>");
																	if(t_5==String(L"1011",4)){
																		DBG_BLOCK();
																		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<406>");
																		bb_LayeredLevel_DrawRoom(String(L"StartRoom/NSW.png",17),t_x,t_y,m_MapPSize,m_MapXOffset);
																	}else{
																		DBG_BLOCK();
																		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<407>");
																		if(t_5==String(L"0101",4)){
																			DBG_BLOCK();
																			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<408>");
																			bb_LayeredLevel_DrawRoom(String(L"StartRoom/EW.png",16),t_x,t_y,m_MapPSize,m_MapXOffset);
																		}else{
																			DBG_BLOCK();
																			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<409>");
																			if(t_5==String(L"1111",4)){
																				DBG_BLOCK();
																				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<410>");
																				bb_LayeredLevel_DrawRoom(String(L"StartRoom/NESW.png",18),t_x,t_y,m_MapPSize,m_MapXOffset);
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}else{
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<412>");
					if(t_4==2){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<413>");
						bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<414>");
						bb_graphics_DrawRect(Float(t_x*m_MapPSize+m_MapXOffset),Float(t_y*m_MapPSize),Float(m_MapPSize),Float(m_MapPSize));
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<415>");
						String t_6=m_Map.At(t_x).At(t_y)->p_GetDoors();
						DBG_LOCAL(t_6,"6")
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<416>");
						if(t_6==String(L"1000",4)){
							DBG_BLOCK();
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<417>");
							bb_LayeredLevel_DrawRoom(String(L"Room/N.png",10),t_x,t_y,m_MapPSize,m_MapXOffset);
						}else{
							DBG_BLOCK();
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<418>");
							if(t_6==String(L"0100",4)){
								DBG_BLOCK();
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<419>");
								bb_LayeredLevel_DrawRoom(String(L"Room/E.png",10),t_x,t_y,m_MapPSize,m_MapXOffset);
							}else{
								DBG_BLOCK();
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<420>");
								if(t_6==String(L"0010",4)){
									DBG_BLOCK();
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<421>");
									bb_LayeredLevel_DrawRoom(String(L"Room/S.png",10),t_x,t_y,m_MapPSize,m_MapXOffset);
								}else{
									DBG_BLOCK();
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<422>");
									if(t_6==String(L"0001",4)){
										DBG_BLOCK();
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<423>");
										bb_LayeredLevel_DrawRoom(String(L"Room/W.png",10),t_x,t_y,m_MapPSize,m_MapXOffset);
									}else{
										DBG_BLOCK();
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<424>");
										if(t_6==String(L"1100",4)){
											DBG_BLOCK();
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<425>");
											bb_LayeredLevel_DrawRoom(String(L"Room/NE.png",11),t_x,t_y,m_MapPSize,m_MapXOffset);
										}else{
											DBG_BLOCK();
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<426>");
											if(t_6==String(L"1010",4)){
												DBG_BLOCK();
												DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<427>");
												bb_LayeredLevel_DrawRoom(String(L"Room/NS.png",11),t_x,t_y,m_MapPSize,m_MapXOffset);
											}else{
												DBG_BLOCK();
												DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<428>");
												if(t_6==String(L"1001",4)){
													DBG_BLOCK();
													DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<429>");
													bb_LayeredLevel_DrawRoom(String(L"Room/NW.png",11),t_x,t_y,m_MapPSize,m_MapXOffset);
												}else{
													DBG_BLOCK();
													DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<430>");
													if(t_6==String(L"1110",4)){
														DBG_BLOCK();
														DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<431>");
														bb_LayeredLevel_DrawRoom(String(L"Room/NES.png",12),t_x,t_y,m_MapPSize,m_MapXOffset);
													}else{
														DBG_BLOCK();
														DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<432>");
														if(t_6==String(L"1101",4)){
															DBG_BLOCK();
															DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<433>");
															bb_LayeredLevel_DrawRoom(String(L"Room/NEW.png",12),t_x,t_y,m_MapPSize,m_MapXOffset);
														}else{
															DBG_BLOCK();
															DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<434>");
															if(t_6==String(L"0110",4)){
																DBG_BLOCK();
																DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<435>");
																bb_LayeredLevel_DrawRoom(String(L"Room/ES.png",11),t_x,t_y,m_MapPSize,m_MapXOffset);
															}else{
																DBG_BLOCK();
																DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<436>");
																if(t_6==String(L"0011",4)){
																	DBG_BLOCK();
																	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<437>");
																	bb_LayeredLevel_DrawRoom(String(L"Room/SW.png",11),t_x,t_y,m_MapPSize,m_MapXOffset);
																}else{
																	DBG_BLOCK();
																	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<438>");
																	if(t_6==String(L"0111",4)){
																		DBG_BLOCK();
																		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<439>");
																		bb_LayeredLevel_DrawRoom(String(L"Room/ESW.png",12),t_x,t_y,m_MapPSize,m_MapXOffset);
																	}else{
																		DBG_BLOCK();
																		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<440>");
																		if(t_6==String(L"1011",4)){
																			DBG_BLOCK();
																			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<441>");
																			bb_LayeredLevel_DrawRoom(String(L"Room/NSW.png",12),t_x,t_y,m_MapPSize,m_MapXOffset);
																		}else{
																			DBG_BLOCK();
																			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<442>");
																			if(t_6==String(L"0101",4)){
																				DBG_BLOCK();
																				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<443>");
																				bb_LayeredLevel_DrawRoom(String(L"Room/EW.png",11),t_x,t_y,m_MapPSize,m_MapXOffset);
																			}else{
																				DBG_BLOCK();
																				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<444>");
																				if(t_6==String(L"1111",4)){
																					DBG_BLOCK();
																					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<445>");
																					bb_LayeredLevel_DrawRoom(String(L"Room/NESW.png",13),t_x,t_y,m_MapPSize,m_MapXOffset);
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
void c_RoomMap::mark(){
	Object::mark();
	gc_mark_q(m_Map);
	gc_mark_q(m_currentRoom);
}
String c_RoomMap::debug(){
	String t="(RoomMap)\n";
	t+=dbg_decl("Map",&m_Map);
	t+=dbg_decl("MapHeight",&m_MapHeight);
	t+=dbg_decl("MapWidth",&m_MapWidth);
	t+=dbg_decl("MapPSize",&m_MapPSize);
	t+=dbg_decl("MapXOffset",&m_MapXOffset);
	t+=dbg_decl("roomNum",&m_roomNum);
	t+=dbg_decl("currentRoom",&m_currentRoom);
	return t;
}
int bb_app__updateRate;
void bb_app_SetUpdateRate(int t_hertz){
	DBG_ENTER("SetUpdateRate")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<224>");
	bb_app__updateRate=t_hertz;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<225>");
	bb_app__game->SetUpdateRate(t_hertz);
}
int bb_input_KeyHit(int t_key){
	DBG_ENTER("KeyHit")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/input.monkey<44>");
	int t_=bb_input_device->p_KeyHit(t_key);
	return t_;
}
int bb_app_Millisecs(){
	DBG_ENTER("Millisecs")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/app.monkey<233>");
	int t_=bb_app__game->Millisecs();
	return t_;
}
int bb_random_Seed;
c_Room::c_Room(){
	m_x=0;
	m_y=0;
	m_Type=0;
	m_WallLayout=Array<Array<int > >();
	m_FloorLayout=Array<Array<int > >();
	m_CollisionArray=Array<Array<int > >();
	m_Neighbours=0;
	m_nDoor=String(L"0",1);
	m_sDoor=String(L"0",1);
	m_wDoor=String(L"0",1);
	m_eDoor=String(L"0",1);
	m_RoomSize32=15;
	m_RoomTiles=bb_graphics_LoadImage(String(L"ProtoTileSet.png",16),1,c_Image::m_DefaultFlags);
	m_DoorArray=Array<String >();
}
c_Room* c_Room::m_new(int t_Type,int t_X,int t_Y){
	DBG_ENTER("Room.new")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_Type,"Type")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<76>");
	this->m_x=t_X;
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<77>");
	this->m_y=t_Y;
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<78>");
	this->m_Type=t_Type;
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<93>");
	int t_[]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,2};
	int t_2[]={3,11,11,11,11,11,11,11,11,11,11,11,11,11,4};
	int t_3[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_4[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_5[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_6[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_7[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_8[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_9[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_10[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_11[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_12[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_13[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_14[]={3,7,7,7,7,7,7,7,7,7,7,7,7,7,4};
	int t_15[]={5,10,10,10,10,10,10,10,10,10,10,10,10,10,6};
	Array<int > t_16[]={Array<int >(t_,15),Array<int >(t_2,15),Array<int >(t_3,15),Array<int >(t_4,15),Array<int >(t_5,15),Array<int >(t_6,15),Array<int >(t_7,15),Array<int >(t_8,15),Array<int >(t_9,15),Array<int >(t_10,15),Array<int >(t_11,15),Array<int >(t_12,15),Array<int >(t_13,15),Array<int >(t_14,15),Array<int >(t_15,15)};
	gc_assign(m_WallLayout,Array<Array<int > >(t_16,15));
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<108>");
	int t_17[]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,2};
	int t_18[]={3,9,9,9,9,9,9,9,9,9,9,9,9,9,4};
	int t_19[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_20[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_21[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_22[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_23[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_24[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_25[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_26[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_27[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_28[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_29[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_30[]={3,8,8,8,8,8,8,8,8,8,8,8,8,8,4};
	int t_31[]={5,10,10,10,10,10,10,10,10,10,10,10,10,10,6};
	Array<int > t_32[]={Array<int >(t_17,15),Array<int >(t_18,15),Array<int >(t_19,15),Array<int >(t_20,15),Array<int >(t_21,15),Array<int >(t_22,15),Array<int >(t_23,15),Array<int >(t_24,15),Array<int >(t_25,15),Array<int >(t_26,15),Array<int >(t_27,15),Array<int >(t_28,15),Array<int >(t_29,15),Array<int >(t_30,15),Array<int >(t_31,15)};
	gc_assign(m_FloorLayout,Array<Array<int > >(t_32,15));
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<123>");
	int t_33[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	int t_34[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_35[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_36[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_37[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_38[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_39[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_40[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_41[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_42[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_43[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_44[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_45[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_46[]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	int t_47[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	Array<int > t_48[]={Array<int >(t_33,15),Array<int >(t_34,15),Array<int >(t_35,15),Array<int >(t_36,15),Array<int >(t_37,15),Array<int >(t_38,15),Array<int >(t_39,15),Array<int >(t_40,15),Array<int >(t_41,15),Array<int >(t_42,15),Array<int >(t_43,15),Array<int >(t_44,15),Array<int >(t_45,15),Array<int >(t_46,15),Array<int >(t_47,15)};
	gc_assign(m_CollisionArray,Array<Array<int > >(t_48,15));
	return this;
}
c_Room* c_Room::m_new2(){
	DBG_ENTER("Room.new")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<59>");
	return this;
}
int c_Room::p_UpdateType(int t_Type){
	DBG_ENTER("Room.UpdateType")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_Type,"Type")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<204>");
	this->m_Type=t_Type;
	return 0;
}
int c_Room::p_UpdateNeighbours(int t_Amount){
	DBG_ENTER("Room.UpdateNeighbours")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_Amount,"Amount")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<200>");
	this->m_Neighbours+=t_Amount;
	return 0;
}
int c_Room::p_GetNeighbours(){
	DBG_ENTER("Room.GetNeighbours")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<208>");
	return m_Neighbours;
}
int c_Room::p_GetType(){
	DBG_ENTER("Room.GetType")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<212>");
	return m_Type;
}
int c_Room::p_GetX(){
	DBG_ENTER("Room.GetX")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<216>");
	return this->m_x;
}
int c_Room::p_GetY(){
	DBG_ENTER("Room.GetY")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<220>");
	return this->m_y;
}
int c_Room::p_SetnDoor(String t__nDoor){
	DBG_ENTER("Room.SetnDoor")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t__nDoor,"_nDoor")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<183>");
	this->m_nDoor=t__nDoor;
	return 0;
}
int c_Room::p_SetsDoor(String t__sDoor){
	DBG_ENTER("Room.SetsDoor")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t__sDoor,"_sDoor")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<187>");
	this->m_sDoor=t__sDoor;
	return 0;
}
int c_Room::p_SetwDoor(String t__wDoor){
	DBG_ENTER("Room.SetwDoor")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t__wDoor,"_wDoor")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<195>");
	this->m_wDoor=t__wDoor;
	return 0;
}
int c_Room::p_SeteDoor(String t__eDoor){
	DBG_ENTER("Room.SeteDoor")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t__eDoor,"_eDoor")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<191>");
	this->m_eDoor=t__eDoor;
	return 0;
}
int c_Room::p_Reset(){
	DBG_ENTER("Room.Reset")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<223>");
	this->m_Type=0;
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<224>");
	this->m_Neighbours=0;
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<225>");
	this->m_nDoor=String(L"0",1);
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<226>");
	this->m_eDoor=String(L"0",1);
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<227>");
	this->m_sDoor=String(L"0",1);
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<228>");
	this->m_wDoor=String(L"0",1);
	return 0;
}
int c_Room::p_DrawFloor(){
	DBG_ENTER("Room.DrawFloor")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<127>");
	for(int t_x=0;t_x<=m_RoomSize32-1;t_x=t_x+1){
		DBG_BLOCK();
		DBG_LOCAL(t_x,"x")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<128>");
		for(int t_y=0;t_y<=m_RoomSize32-1;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<129>");
			if(m_FloorLayout.At(t_y).At(t_x)==8){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<130>");
				c_Image* t_floor=m_RoomTiles->p_GrabImage(0,288,32,32,1,c_Image::m_DefaultFlags);
				DBG_LOCAL(t_floor,"floor")
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<131>");
				bb_graphics_DrawImage(t_floor,Float(t_x*32),Float(t_y*32),0);
			}
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<133>");
			if(m_FloorLayout.At(t_y).At(t_x)==9){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<134>");
				c_Image* t_topBrick=m_RoomTiles->p_GrabImage(32,352,32,32,1,c_Image::m_DefaultFlags);
				DBG_LOCAL(t_topBrick,"topBrick")
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<135>");
				bb_graphics_DrawImage(t_topBrick,Float(t_x*32),Float(t_y*32),0);
			}
		}
	}
	return 0;
}
int c_Room::p_DrawWalls(){
	DBG_ENTER("Room.DrawWalls")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<142>");
	for(int t_x=0;t_x<=m_RoomSize32-1;t_x=t_x+1){
		DBG_BLOCK();
		DBG_LOCAL(t_x,"x")
		DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<143>");
		for(int t_y=0;t_y<=m_RoomSize32-1;t_y=t_y+1){
			DBG_BLOCK();
			DBG_LOCAL(t_y,"y")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<144>");
			int t_3=m_WallLayout.At(t_y).At(t_x);
			DBG_LOCAL(t_3,"3")
			DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<145>");
			if(t_3==0){
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<146>");
				c_Image* t_topLeftWall=m_RoomTiles->p_GrabImage(0,320,32,32,1,c_Image::m_DefaultFlags);
				DBG_LOCAL(t_topLeftWall,"topLeftWall")
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<147>");
				bb_graphics_DrawImage(t_topLeftWall,Float(t_x*32),Float(t_y*32),0);
			}else{
				DBG_BLOCK();
				DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<148>");
				if(t_3==1){
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<149>");
					c_Image* t_wall=m_RoomTiles->p_GrabImage(32,320,32,32,1,c_Image::m_DefaultFlags);
					DBG_LOCAL(t_wall,"wall")
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<150>");
					bb_graphics_DrawImage(t_wall,Float(t_x*32),Float(t_y*32),0);
				}else{
					DBG_BLOCK();
					DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<151>");
					if(t_3==2){
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<152>");
						c_Image* t_topRightWall=m_RoomTiles->p_GrabImage(64,320,32,32,1,c_Image::m_DefaultFlags);
						DBG_LOCAL(t_topRightWall,"topRightWall")
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<153>");
						bb_graphics_DrawImage(t_topRightWall,Float(t_x*32),Float(t_y*32),0);
					}else{
						DBG_BLOCK();
						DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<154>");
						if(t_3==3){
							DBG_BLOCK();
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<155>");
							c_Image* t_leftWall=m_RoomTiles->p_GrabImage(0,352,32,32,1,c_Image::m_DefaultFlags);
							DBG_LOCAL(t_leftWall,"leftWall")
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<156>");
							bb_graphics_DrawImage(t_leftWall,Float(t_x*32),Float(t_y*32),0);
						}else{
							DBG_BLOCK();
							DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<157>");
							if(t_3==4){
								DBG_BLOCK();
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<158>");
								c_Image* t_rightWall=m_RoomTiles->p_GrabImage(64,352,32,32,1,c_Image::m_DefaultFlags);
								DBG_LOCAL(t_rightWall,"rightWall")
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<159>");
								bb_graphics_DrawImage(t_rightWall,Float(t_x*32),Float(t_y*32),0);
							}else{
								DBG_BLOCK();
								DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<160>");
								if(t_3==5){
									DBG_BLOCK();
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<161>");
									c_Image* t_botLeftWall=m_RoomTiles->p_GrabImage(0,416,32,32,1,c_Image::m_DefaultFlags);
									DBG_LOCAL(t_botLeftWall,"botLeftWall")
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<162>");
									bb_graphics_DrawImage(t_botLeftWall,Float(t_x*32),Float(t_y*32),0);
								}else{
									DBG_BLOCK();
									DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<163>");
									if(t_3==6){
										DBG_BLOCK();
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<164>");
										c_Image* t_botRightWall=m_RoomTiles->p_GrabImage(64,416,32,32,1,c_Image::m_DefaultFlags);
										DBG_LOCAL(t_botRightWall,"botRightWall")
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<165>");
										bb_graphics_DrawImage(t_botRightWall,Float(t_x*32),Float(t_y*32),0);
									}else{
										DBG_BLOCK();
										DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<166>");
										if(t_3==7){
											DBG_BLOCK();
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<167>");
											c_Image* t_botWall=m_RoomTiles->p_GrabImage(32,384,32,32,1,c_Image::m_DefaultFlags);
											DBG_LOCAL(t_botWall,"botWall")
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<168>");
											bb_graphics_DrawImage(t_botWall,Float(t_x*32),Float(t_y*32),0);
										}else{
											DBG_BLOCK();
											DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<169>");
											if(t_3==10){
												DBG_BLOCK();
												DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<170>");
												c_Image* t_botBrick=m_RoomTiles->p_GrabImage(32,416,32,32,1,c_Image::m_DefaultFlags);
												DBG_LOCAL(t_botBrick,"botBrick")
												DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<171>");
												bb_graphics_DrawImage(t_botBrick,Float(t_x*32),Float(t_y*32),0);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
String c_Room::p_GetDoors(){
	DBG_ENTER("Room.GetDoors")
	c_Room *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<178>");
	String t_[]={m_nDoor,m_eDoor,m_sDoor,m_wDoor};
	gc_assign(m_DoorArray,Array<String >(t_,4));
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<179>");
	String t_2=String().Join(m_DoorArray);
	return t_2;
}
void c_Room::mark(){
	Object::mark();
	gc_mark_q(m_WallLayout);
	gc_mark_q(m_FloorLayout);
	gc_mark_q(m_CollisionArray);
	gc_mark_q(m_RoomTiles);
	gc_mark_q(m_DoorArray);
}
String c_Room::debug(){
	String t="(Room)\n";
	t+=dbg_decl("Type",&m_Type);
	t+=dbg_decl("Neighbours",&m_Neighbours);
	t+=dbg_decl("x",&m_x);
	t+=dbg_decl("y",&m_y);
	t+=dbg_decl("nDoor",&m_nDoor);
	t+=dbg_decl("eDoor",&m_eDoor);
	t+=dbg_decl("wDoor",&m_wDoor);
	t+=dbg_decl("sDoor",&m_sDoor);
	t+=dbg_decl("DoorArray",&m_DoorArray);
	t+=dbg_decl("RoomTiles",&m_RoomTiles);
	t+=dbg_decl("WallLayout",&m_WallLayout);
	t+=dbg_decl("FloorLayout",&m_FloorLayout);
	t+=dbg_decl("CollisionArray",&m_CollisionArray);
	t+=dbg_decl("RoomSize32",&m_RoomSize32);
	return t;
}
Float bb_random_Rnd(){
	DBG_ENTER("Rnd")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/random.monkey<21>");
	bb_random_Seed=bb_random_Seed*1664525+1013904223|0;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/random.monkey<22>");
	Float t_=Float(bb_random_Seed>>8&16777215)/FLOAT(16777216.0);
	return t_;
}
Float bb_random_Rnd2(Float t_low,Float t_high){
	DBG_ENTER("Rnd")
	DBG_LOCAL(t_low,"low")
	DBG_LOCAL(t_high,"high")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/random.monkey<30>");
	Float t_=bb_random_Rnd3(t_high-t_low)+t_low;
	return t_;
}
Float bb_random_Rnd3(Float t_range){
	DBG_ENTER("Rnd")
	DBG_LOCAL(t_range,"range")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/random.monkey<26>");
	Float t_=bb_random_Rnd()*t_range;
	return t_;
}
c_List::c_List(){
	m__head=((new c_HeadNode)->m_new());
}
c_List* c_List::m_new(){
	DBG_ENTER("List.new")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
c_Node2* c_List::p_AddLast(c_Room* t_data){
	DBG_ENTER("List.AddLast")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<108>");
	c_Node2* t_=(new c_Node2)->m_new(m__head,m__head->m__pred,t_data);
	return t_;
}
c_List* c_List::m_new2(Array<c_Room* > t_data){
	DBG_ENTER("List.new")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<13>");
	Array<c_Room* > t_=t_data;
	int t_2=0;
	while(t_2<t_.Length()){
		DBG_BLOCK();
		c_Room* t_t=t_.At(t_2);
		t_2=t_2+1;
		DBG_LOCAL(t_t,"t")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<14>");
		p_AddLast(t_t);
	}
	return this;
}
int c_List::p_Count(){
	DBG_ENTER("List.Count")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<41>");
	int t_n=0;
	c_Node2* t_node=m__head->m__succ;
	DBG_LOCAL(t_n,"n")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<42>");
	while(t_node!=m__head){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<43>");
		t_node=t_node->m__succ;
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<44>");
		t_n+=1;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<46>");
	return t_n;
}
c_Enumerator* c_List::p_ObjectEnumerator(){
	DBG_ENTER("List.ObjectEnumerator")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<186>");
	c_Enumerator* t_=(new c_Enumerator)->m_new(this);
	return t_;
}
Array<c_Room* > c_List::p_ToArray(){
	DBG_ENTER("List.ToArray")
	c_List *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<19>");
	Array<c_Room* > t_arr=Array<c_Room* >(p_Count());
	int t_i=0;
	DBG_LOCAL(t_arr,"arr")
	DBG_LOCAL(t_i,"i")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<20>");
	c_Enumerator* t_=this->p_ObjectEnumerator();
	while(t_->p_HasNext()){
		DBG_BLOCK();
		c_Room* t_t=t_->p_NextObject();
		DBG_LOCAL(t_t,"t")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<21>");
		gc_assign(t_arr.At(t_i),t_t);
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<22>");
		t_i+=1;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<24>");
	return t_arr;
}
void c_List::mark(){
	Object::mark();
	gc_mark_q(m__head);
}
String c_List::debug(){
	String t="(List)\n";
	t+=dbg_decl("_head",&m__head);
	return t;
}
c_Node2::c_Node2(){
	m__succ=0;
	m__pred=0;
	m__data=0;
}
c_Node2* c_Node2::m_new(c_Node2* t_succ,c_Node2* t_pred,c_Room* t_data){
	DBG_ENTER("Node.new")
	c_Node2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_succ,"succ")
	DBG_LOCAL(t_pred,"pred")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<261>");
	gc_assign(m__succ,t_succ);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<262>");
	gc_assign(m__pred,t_pred);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<263>");
	gc_assign(m__succ->m__pred,this);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<264>");
	gc_assign(m__pred->m__succ,this);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<265>");
	gc_assign(m__data,t_data);
	return this;
}
c_Node2* c_Node2::m_new2(){
	DBG_ENTER("Node.new")
	c_Node2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<258>");
	return this;
}
void c_Node2::mark(){
	Object::mark();
	gc_mark_q(m__succ);
	gc_mark_q(m__pred);
	gc_mark_q(m__data);
}
String c_Node2::debug(){
	String t="(Node)\n";
	t+=dbg_decl("_succ",&m__succ);
	t+=dbg_decl("_pred",&m__pred);
	t+=dbg_decl("_data",&m__data);
	return t;
}
c_HeadNode::c_HeadNode(){
}
c_HeadNode* c_HeadNode::m_new(){
	DBG_ENTER("HeadNode.new")
	c_HeadNode *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<310>");
	c_Node2::m_new2();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<311>");
	gc_assign(m__succ,(this));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<312>");
	gc_assign(m__pred,(this));
	return this;
}
void c_HeadNode::mark(){
	c_Node2::mark();
}
String c_HeadNode::debug(){
	String t="(HeadNode)\n";
	t=c_Node2::debug()+t;
	return t;
}
c_Enumerator::c_Enumerator(){
	m__list=0;
	m__curr=0;
}
c_Enumerator* c_Enumerator::m_new(c_List* t_list){
	DBG_ENTER("Enumerator.new")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_list,"list")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<326>");
	gc_assign(m__list,t_list);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<327>");
	gc_assign(m__curr,t_list->m__head->m__succ);
	return this;
}
c_Enumerator* c_Enumerator::m_new2(){
	DBG_ENTER("Enumerator.new")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<323>");
	return this;
}
bool c_Enumerator::p_HasNext(){
	DBG_ENTER("Enumerator.HasNext")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<331>");
	while(m__curr->m__succ->m__pred!=m__curr){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<332>");
		gc_assign(m__curr,m__curr->m__succ);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<334>");
	bool t_=m__curr!=m__list->m__head;
	return t_;
}
c_Room* c_Enumerator::p_NextObject(){
	DBG_ENTER("Enumerator.NextObject")
	c_Enumerator *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<338>");
	c_Room* t_data=m__curr->m__data;
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<339>");
	gc_assign(m__curr,m__curr->m__succ);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/monkey/list.monkey<340>");
	return t_data;
}
void c_Enumerator::mark(){
	Object::mark();
	gc_mark_q(m__list);
	gc_mark_q(m__curr);
}
String c_Enumerator::debug(){
	String t="(Enumerator)\n";
	t+=dbg_decl("_list",&m__list);
	t+=dbg_decl("_curr",&m__curr);
	return t;
}
int bb_graphics_DebugRenderDevice(){
	DBG_ENTER("DebugRenderDevice")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<53>");
	if(!((bb_graphics_renderDevice)!=0)){
		DBG_BLOCK();
		bbError(String(L"Rendering operations can only be performed inside OnRender",58));
	}
	return 0;
}
int bb_graphics_Cls(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("Cls")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<382>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<384>");
	bb_graphics_renderDevice->Cls(t_r,t_g,t_b);
	return 0;
}
int bb_graphics_DrawImage(c_Image* t_image,Float t_x,Float t_y,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<455>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<456>");
	if(t_frame<0 || t_frame>=t_image->m_frames.Length()){
		DBG_BLOCK();
		bbError(String(L"Invalid image frame",19));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<459>");
	c_Frame* t_f=t_image->m_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<461>");
	bb_graphics_context->p_Validate();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<463>");
	if((t_image->m_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<464>");
		bb_graphics_renderDevice->DrawSurface(t_image->m_surface,t_x-t_image->m_tx,t_y-t_image->m_ty);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<466>");
		bb_graphics_renderDevice->DrawSurface2(t_image->m_surface,t_x-t_image->m_tx,t_y-t_image->m_ty,t_f->m_x,t_f->m_y,t_image->m_width,t_image->m_height);
	}
	return 0;
}
int bb_graphics_PushMatrix(){
	DBG_ENTER("PushMatrix")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<338>");
	int t_sp=bb_graphics_context->m_matrixSp;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<339>");
	bb_graphics_context->m_matrixStack.At(t_sp+0)=bb_graphics_context->m_ix;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<340>");
	bb_graphics_context->m_matrixStack.At(t_sp+1)=bb_graphics_context->m_iy;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<341>");
	bb_graphics_context->m_matrixStack.At(t_sp+2)=bb_graphics_context->m_jx;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<342>");
	bb_graphics_context->m_matrixStack.At(t_sp+3)=bb_graphics_context->m_jy;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<343>");
	bb_graphics_context->m_matrixStack.At(t_sp+4)=bb_graphics_context->m_tx;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<344>");
	bb_graphics_context->m_matrixStack.At(t_sp+5)=bb_graphics_context->m_ty;
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<345>");
	bb_graphics_context->m_matrixSp=t_sp+6;
	return 0;
}
int bb_graphics_Transform(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<359>");
	Float t_ix2=t_ix*bb_graphics_context->m_ix+t_iy*bb_graphics_context->m_jx;
	DBG_LOCAL(t_ix2,"ix2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<360>");
	Float t_iy2=t_ix*bb_graphics_context->m_iy+t_iy*bb_graphics_context->m_jy;
	DBG_LOCAL(t_iy2,"iy2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<361>");
	Float t_jx2=t_jx*bb_graphics_context->m_ix+t_jy*bb_graphics_context->m_jx;
	DBG_LOCAL(t_jx2,"jx2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<362>");
	Float t_jy2=t_jx*bb_graphics_context->m_iy+t_jy*bb_graphics_context->m_jy;
	DBG_LOCAL(t_jy2,"jy2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<363>");
	Float t_tx2=t_tx*bb_graphics_context->m_ix+t_ty*bb_graphics_context->m_jx+bb_graphics_context->m_tx;
	DBG_LOCAL(t_tx2,"tx2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<364>");
	Float t_ty2=t_tx*bb_graphics_context->m_iy+t_ty*bb_graphics_context->m_jy+bb_graphics_context->m_ty;
	DBG_LOCAL(t_ty2,"ty2")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<365>");
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	return 0;
}
int bb_graphics_Transform2(Array<Float > t_m){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<355>");
	bb_graphics_Transform(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_Translate(Float t_x,Float t_y){
	DBG_ENTER("Translate")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<369>");
	bb_graphics_Transform(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),t_x,t_y);
	return 0;
}
int bb_graphics_Rotate(Float t_angle){
	DBG_ENTER("Rotate")
	DBG_LOCAL(t_angle,"angle")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<377>");
	bb_graphics_Transform((Float)cos((t_angle)*D2R),-(Float)sin((t_angle)*D2R),(Float)sin((t_angle)*D2R),(Float)cos((t_angle)*D2R),FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_Scale(Float t_x,Float t_y){
	DBG_ENTER("Scale")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<373>");
	bb_graphics_Transform(t_x,FLOAT(0.0),FLOAT(0.0),t_y,FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_PopMatrix(){
	DBG_ENTER("PopMatrix")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<349>");
	int t_sp=bb_graphics_context->m_matrixSp-6;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<350>");
	bb_graphics_SetMatrix(bb_graphics_context->m_matrixStack.At(t_sp+0),bb_graphics_context->m_matrixStack.At(t_sp+1),bb_graphics_context->m_matrixStack.At(t_sp+2),bb_graphics_context->m_matrixStack.At(t_sp+3),bb_graphics_context->m_matrixStack.At(t_sp+4),bb_graphics_context->m_matrixStack.At(t_sp+5));
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<351>");
	bb_graphics_context->m_matrixSp=t_sp;
	return 0;
}
int bb_graphics_DrawImage2(c_Image* t_image,Float t_x,Float t_y,Float t_rotation,Float t_scaleX,Float t_scaleY,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_rotation,"rotation")
	DBG_LOCAL(t_scaleX,"scaleX")
	DBG_LOCAL(t_scaleY,"scaleY")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<473>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<474>");
	if(t_frame<0 || t_frame>=t_image->m_frames.Length()){
		DBG_BLOCK();
		bbError(String(L"Invalid image frame",19));
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<477>");
	c_Frame* t_f=t_image->m_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<479>");
	bb_graphics_PushMatrix();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<481>");
	bb_graphics_Translate(t_x,t_y);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<482>");
	bb_graphics_Rotate(t_rotation);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<483>");
	bb_graphics_Scale(t_scaleX,t_scaleY);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<485>");
	bb_graphics_Translate(-t_image->m_tx,-t_image->m_ty);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<487>");
	bb_graphics_context->p_Validate();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<489>");
	if((t_image->m_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<490>");
		bb_graphics_renderDevice->DrawSurface(t_image->m_surface,FLOAT(0.0),FLOAT(0.0));
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<492>");
		bb_graphics_renderDevice->DrawSurface2(t_image->m_surface,FLOAT(0.0),FLOAT(0.0),t_f->m_x,t_f->m_y,t_image->m_width,t_image->m_height);
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<495>");
	bb_graphics_PopMatrix();
	return 0;
}
int bb_graphics_DrawText(String t_text,Float t_x,Float t_y,Float t_xalign,Float t_yalign){
	DBG_ENTER("DrawText")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_xalign,"xalign")
	DBG_LOCAL(t_yalign,"yalign")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<580>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<582>");
	if(!((bb_graphics_context->m_font)!=0)){
		DBG_BLOCK();
		return 0;
	}
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<584>");
	int t_w=bb_graphics_context->m_font->p_Width();
	DBG_LOCAL(t_w,"w")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<585>");
	int t_h=bb_graphics_context->m_font->p_Height();
	DBG_LOCAL(t_h,"h")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<587>");
	t_x-=(Float)floor(Float(t_w*t_text.Length())*t_xalign);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<588>");
	t_y-=(Float)floor(Float(t_h)*t_yalign);
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<590>");
	for(int t_i=0;t_i<t_text.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<591>");
		int t_ch=(int)t_text.At(t_i)-bb_graphics_context->m_firstChar;
		DBG_LOCAL(t_ch,"ch")
		DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<592>");
		if(t_ch>=0 && t_ch<bb_graphics_context->m_font->p_Frames()){
			DBG_BLOCK();
			DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<593>");
			bb_graphics_DrawImage(bb_graphics_context->m_font,t_x+Float(t_i*t_w),t_y,t_ch);
		}
	}
	return 0;
}
int bb_graphics_DrawRect(Float t_x,Float t_y,Float t_w,Float t_h){
	DBG_ENTER("DrawRect")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<397>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<399>");
	bb_graphics_context->p_Validate();
	DBG_INFO("C:/Users/camjr/Documents/MonkeyX/JungleMonkey78b/modules/mojo/graphics.monkey<400>");
	bb_graphics_renderDevice->DrawRect(t_x,t_y,t_w,t_h);
	return 0;
}
int bb_LayeredLevel_DrawRoom(String t_path,int t_x,int t_y,int t_mapPSize,int t_mapXOffset){
	DBG_ENTER("DrawRoom")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_mapPSize,"mapPSize")
	DBG_LOCAL(t_mapXOffset,"mapXOffset")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<464>");
	c_Image* t_Room=0;
	DBG_LOCAL(t_Room,"Room")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<465>");
	int t_ImageSize=32;
	DBG_LOCAL(t_ImageSize,"ImageSize")
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<466>");
	t_Room=bb_graphics_LoadImage2(t_path,t_ImageSize,t_ImageSize,1,c_Image::m_DefaultFlags);
	DBG_INFO("F:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<467>");
	bb_graphics_DrawImage(t_Room,Float(t_x*t_mapPSize+t_mapXOffset),Float(t_y*t_mapPSize),0);
	return 0;
}
int bbInit(){
	GC_CTOR
	bb_app__app=0;
	DBG_GLOBAL("_app",&bb_app__app);
	bb_app__delegate=0;
	DBG_GLOBAL("_delegate",&bb_app__delegate);
	bb_app__game=BBGame::Game();
	DBG_GLOBAL("_game",&bb_app__game);
	bb_LayeredLevel_Prototype=0;
	DBG_GLOBAL("Prototype",&bb_LayeredLevel_Prototype);
	bb_graphics_device=0;
	DBG_GLOBAL("device",&bb_graphics_device);
	bb_graphics_context=(new c_GraphicsContext)->m_new();
	DBG_GLOBAL("context",&bb_graphics_context);
	c_Image::m_DefaultFlags=0;
	DBG_GLOBAL("DefaultFlags",&c_Image::m_DefaultFlags);
	bb_audio_device=0;
	DBG_GLOBAL("device",&bb_audio_device);
	bb_input_device=0;
	DBG_GLOBAL("device",&bb_input_device);
	bb_app__devWidth=0;
	DBG_GLOBAL("_devWidth",&bb_app__devWidth);
	bb_app__devHeight=0;
	DBG_GLOBAL("_devHeight",&bb_app__devHeight);
	bb_app__displayModes=Array<c_DisplayMode* >();
	DBG_GLOBAL("_displayModes",&bb_app__displayModes);
	bb_app__desktopMode=0;
	DBG_GLOBAL("_desktopMode",&bb_app__desktopMode);
	bb_graphics_renderDevice=0;
	DBG_GLOBAL("renderDevice",&bb_graphics_renderDevice);
	bb_app__updateRate=0;
	DBG_GLOBAL("_updateRate",&bb_app__updateRate);
	bb_random_Seed=1234;
	DBG_GLOBAL("Seed",&bb_random_Seed);
	return 0;
}
void gc_mark(){
	gc_mark_q(bb_app__app);
	gc_mark_q(bb_app__delegate);
	gc_mark_q(bb_LayeredLevel_Prototype);
	gc_mark_q(bb_graphics_device);
	gc_mark_q(bb_graphics_context);
	gc_mark_q(bb_audio_device);
	gc_mark_q(bb_input_device);
	gc_mark_q(bb_app__displayModes);
	gc_mark_q(bb_app__desktopMode);
	gc_mark_q(bb_graphics_renderDevice);
}
//${TRANSCODE_END}

int main( int argc,const char *argv[] ){

	BBMonkeyGame::Main( argc,argv );
}
