
//${CONFIG_BEGIN}
CFG_BINARY_FILES="*.bin|*.dat";
CFG_BRL_DATABUFFER_IMPLEMENTED="1";
CFG_BRL_GAMETARGET_IMPLEMENTED="1";
CFG_BRL_STREAM_IMPLEMENTED="1";
CFG_BRL_THREAD_IMPLEMENTED="1";
CFG_CD="";
CFG_CONFIG="debug";
CFG_HOST="winnt";
CFG_HTML5_WEBAUDIO_ENABLED="1";
CFG_IMAGE_FILES="*.png|*.jpg";
CFG_LANG="js";
CFG_MODPATH="";
CFG_MOJO_AUTO_SUSPEND_ENABLED="1";
CFG_MOJO_DRIVER_IMPLEMENTED="1";
CFG_MUSIC_FILES="*.wav|*.ogg|*.mp3|*.m4a";
CFG_OPENGL_GLES20_ENABLED="0";
CFG_SAFEMODE="0";
CFG_SOUND_FILES="*.wav|*.ogg|*.mp3|*.m4a";
CFG_TARGET="html5";
CFG_TEXT_FILES="*.txt|*.xml|*.json";
//${CONFIG_END}

//${METADATA_BEGIN}
var META_DATA="[mojo_font.png];type=image/png;width=864;height=13;\n[PlayerDirectionTest.png];type=image/png;width=160;height=32;\n[ProtoTileSet.png];type=image/png;width=512;height=512;\n[Room/E.png];type=image/png;width=32;height=32;\n[Room/ES.png];type=image/png;width=32;height=32;\n[Room/ESW.png];type=image/png;width=32;height=32;\n[Room/EW.png];type=image/png;width=32;height=32;\n[Room/N.png];type=image/png;width=32;height=32;\n[Room/NE.png];type=image/png;width=32;height=32;\n[Room/NES.png];type=image/png;width=32;height=32;\n[Room/NESW.png];type=image/png;width=32;height=32;\n[Room/NEW.png];type=image/png;width=32;height=32;\n[Room/NS.png];type=image/png;width=32;height=32;\n[Room/NSW.png];type=image/png;width=32;height=32;\n[Room/NW.png];type=image/png;width=32;height=32;\n[Room/S.png];type=image/png;width=32;height=32;\n[Room/SW.png];type=image/png;width=32;height=32;\n[Room/W.png];type=image/png;width=32;height=32;\n[StartRoom/E.png];type=image/png;width=32;height=32;\n[StartRoom/ES.png];type=image/png;width=32;height=32;\n[StartRoom/ESW.png];type=image/png;width=32;height=32;\n[StartRoom/EW.png];type=image/png;width=32;height=32;\n[StartRoom/N.png];type=image/png;width=32;height=32;\n[StartRoom/NE.png];type=image/png;width=32;height=32;\n[StartRoom/NES.png];type=image/png;width=32;height=32;\n[StartRoom/NESW.png];type=image/png;width=32;height=32;\n[StartRoom/NEW.png];type=image/png;width=32;height=32;\n[StartRoom/NS.png];type=image/png;width=32;height=32;\n[StartRoom/NSW.png];type=image/png;width=32;height=32;\n[StartRoom/NW.png];type=image/png;width=32;height=32;\n[StartRoom/S.png];type=image/png;width=32;height=32;\n[StartRoom/SW.png];type=image/png;width=32;height=32;\n[StartRoom/W.png];type=image/png;width=32;height=32;\n";
//${METADATA_END}

//${TRANSCODE_BEGIN}

// Javascript Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.

//***** JavaScript Runtime *****

var D2R=0.017453292519943295;
var R2D=57.29577951308232;

var err_info="";
var err_stack=[];

var dbg_index=0;

function push_err(){
	err_stack.push( err_info );
}

function pop_err(){
	err_info=err_stack.pop();
}

function stackTrace(){
	if( !err_info.length ) return "";
	var str=err_info+"\n";
	for( var i=err_stack.length-1;i>0;--i ){
		str+=err_stack[i]+"\n";
	}
	return str;
}

function print( str ){
	var cons=document.getElementById( "GameConsole" );
	if( cons ){
		cons.value+=str+"\n";
		cons.scrollTop=cons.scrollHeight-cons.clientHeight;
	}else if( window.console!=undefined ){
		window.console.log( str );
	}
	return 0;
}

function alertError( err ){
	if( typeof(err)=="string" && err=="" ) return;
	alert( "Monkey Runtime Error : "+err.toString()+"\n\n"+stackTrace() );
}

function error( err ){
	throw err;
}

function debugLog( str ){
	if( window.console!=undefined ) window.console.log( str );
}

function debugStop(){
	debugger;	//	error( "STOP" );
}

function dbg_object( obj ){
	if( obj ) return obj;
	error( "Null object access" );
}

function dbg_charCodeAt( str,index ){
	if( index<0 || index>=str.length ) error( "Character index out of range" );
	return str.charCodeAt( index );
}

function dbg_array( arr,index ){
	if( index<0 || index>=arr.length ) error( "Array index out of range" );
	dbg_index=index;
	return arr;
}

function new_bool_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=false;
	return arr;
}

function new_number_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=0;
	return arr;
}

function new_string_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]='';
	return arr;
}

function new_array_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=[];
	return arr;
}

function new_object_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=null;
	return arr;
}

function resize_bool_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=false;
	return arr;
}

function resize_number_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=0;
	return arr;
}

function resize_string_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]="";
	return arr;
}

function resize_array_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=[];
	return arr;
}

function resize_object_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=null;
	return arr;
}

function string_compare( lhs,rhs ){
	var n=Math.min( lhs.length,rhs.length ),i,t;
	for( i=0;i<n;++i ){
		t=lhs.charCodeAt(i)-rhs.charCodeAt(i);
		if( t ) return t;
	}
	return lhs.length-rhs.length;
}

function string_replace( str,find,rep ){	//no unregex replace all?!?
	var i=0;
	for(;;){
		i=str.indexOf( find,i );
		if( i==-1 ) return str;
		str=str.substring( 0,i )+rep+str.substring( i+find.length );
		i+=rep.length;
	}
}

function string_trim( str ){
	var i=0,i2=str.length;
	while( i<i2 && str.charCodeAt(i)<=32 ) i+=1;
	while( i2>i && str.charCodeAt(i2-1)<=32 ) i2-=1;
	return str.slice( i,i2 );
}

function string_startswith( str,substr ){
	return substr.length<=str.length && str.slice(0,substr.length)==substr;
}

function string_endswith( str,substr ){
	return substr.length<=str.length && str.slice(str.length-substr.length,str.length)==substr;
}

function string_tochars( str ){
	var arr=new Array( str.length );
	for( var i=0;i<str.length;++i ) arr[i]=str.charCodeAt(i);
	return arr;
}

function string_fromchars( chars ){
	var str="",i;
	for( i=0;i<chars.length;++i ){
		str+=String.fromCharCode( chars[i] );
	}
	return str;
}

function object_downcast( obj,clas ){
	if( obj instanceof clas ) return obj;
	return null;
}

function object_implements( obj,iface ){
	if( obj && obj.implments && obj.implments[iface] ) return obj;
	return null;
}

function extend_class( clas ){
	var tmp=function(){};
	tmp.prototype=clas.prototype;
	return new tmp;
}

function ThrowableObject(){
}

ThrowableObject.prototype.toString=function(){ 
	return "Uncaught Monkey Exception"; 
}


function BBGameEvent(){}
BBGameEvent.KeyDown=1;
BBGameEvent.KeyUp=2;
BBGameEvent.KeyChar=3;
BBGameEvent.MouseDown=4;
BBGameEvent.MouseUp=5;
BBGameEvent.MouseMove=6;
BBGameEvent.TouchDown=7;
BBGameEvent.TouchUp=8;
BBGameEvent.TouchMove=9;
BBGameEvent.MotionAccel=10;

function BBGameDelegate(){}
BBGameDelegate.prototype.StartGame=function(){}
BBGameDelegate.prototype.SuspendGame=function(){}
BBGameDelegate.prototype.ResumeGame=function(){}
BBGameDelegate.prototype.UpdateGame=function(){}
BBGameDelegate.prototype.RenderGame=function(){}
BBGameDelegate.prototype.KeyEvent=function( ev,data ){}
BBGameDelegate.prototype.MouseEvent=function( ev,data,x,y ){}
BBGameDelegate.prototype.TouchEvent=function( ev,data,x,y ){}
BBGameDelegate.prototype.MotionEvent=function( ev,data,x,y,z ){}
BBGameDelegate.prototype.DiscardGraphics=function(){}

function BBDisplayMode( width,height ){
	this.width=width;
	this.height=height;
}

function BBGame(){
	BBGame._game=this;
	this._delegate=null;
	this._keyboardEnabled=false;
	this._updateRate=0;
	this._started=false;
	this._suspended=false;
	this._debugExs=(CFG_CONFIG=="debug");
	this._startms=Date.now();
}

BBGame.Game=function(){
	return BBGame._game;
}

BBGame.prototype.SetDelegate=function( delegate ){
	this._delegate=delegate;
}

BBGame.prototype.Delegate=function(){
	return this._delegate;
}

BBGame.prototype.SetUpdateRate=function( updateRate ){
	this._updateRate=updateRate;
}

BBGame.prototype.SetKeyboardEnabled=function( keyboardEnabled ){
	this._keyboardEnabled=keyboardEnabled;
}

BBGame.prototype.Started=function(){
	return this._started;
}

BBGame.prototype.Suspended=function(){
	return this._suspended;
}

BBGame.prototype.Millisecs=function(){
	return Date.now()-this._startms;
}

BBGame.prototype.GetDate=function( date ){
	var n=date.length;
	if( n>0 ){
		var t=new Date();
		date[0]=t.getFullYear();
		if( n>1 ){
			date[1]=t.getMonth()+1;
			if( n>2 ){
				date[2]=t.getDate();
				if( n>3 ){
					date[3]=t.getHours();
					if( n>4 ){
						date[4]=t.getMinutes();
						if( n>5 ){
							date[5]=t.getSeconds();
							if( n>6 ){
								date[6]=t.getMilliseconds();
							}
						}
					}
				}
			}
		}
	}
}

BBGame.prototype.SaveState=function( state ){
	localStorage.setItem( "monkeystate@"+document.URL,state );	//key can't start with dot in Chrome!
	return 1;
}

BBGame.prototype.LoadState=function(){
	var state=localStorage.getItem( "monkeystate@"+document.URL );
	if( state ) return state;
	return "";
}

BBGame.prototype.LoadString=function( path ){

	var xhr=new XMLHttpRequest();
	xhr.open( "GET",this.PathToUrl( path ),false );
	
	xhr.send( null );
	
	if( xhr.status==200 || xhr.status==0 ) return xhr.responseText;
	
	return "";
}

BBGame.prototype.PollJoystick=function( port,joyx,joyy,joyz,buttons ){
	return false;
}

BBGame.prototype.OpenUrl=function( url ){
	window.location=url;
}

BBGame.prototype.SetMouseVisible=function( visible ){
	if( visible ){
		this._canvas.style.cursor='default';	
	}else{
		this._canvas.style.cursor="url('data:image/cur;base64,AAACAAEAICAAAAAAAACoEAAAFgAAACgAAAAgAAAAQAAAAAEAIAAAAAAAgBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA55ZXBgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOeWVxAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADnllcGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////9////////////////////+////////f/////////8%3D'), auto";
	}
}

BBGame.prototype.GetDeviceWidth=function(){
	return 0;
}

BBGame.prototype.GetDeviceHeight=function(){
	return 0;
}

BBGame.prototype.SetDeviceWindow=function( width,height,flags ){
}

BBGame.prototype.GetDisplayModes=function(){
	return new Array();
}

BBGame.prototype.GetDesktopMode=function(){
	return null;
}

BBGame.prototype.SetSwapInterval=function( interval ){
}

BBGame.prototype.PathToFilePath=function( path ){
	return "";
}

//***** js Game *****

BBGame.prototype.PathToUrl=function( path ){
	return path;
}

BBGame.prototype.LoadData=function( path ){

	var xhr=new XMLHttpRequest();
	xhr.open( "GET",this.PathToUrl( path ),false );

	if( xhr.overrideMimeType ) xhr.overrideMimeType( "text/plain; charset=x-user-defined" );

	xhr.send( null );
	if( xhr.status!=200 && xhr.status!=0 ) return null;

	var r=xhr.responseText;
	var buf=new ArrayBuffer( r.length );
	var bytes=new Int8Array( buf );
	for( var i=0;i<r.length;++i ){
		bytes[i]=r.charCodeAt( i );
	}
	return buf;
}

//***** INTERNAL ******

BBGame.prototype.Die=function( ex ){

	this._delegate=new BBGameDelegate();
	
	if( !ex.toString() ){
		return;
	}
	
	if( this._debugExs ){
		print( "Monkey Runtime Error : "+ex.toString() );
		print( stackTrace() );
	}
	
	throw ex;
}

BBGame.prototype.StartGame=function(){

	if( this._started ) return;
	this._started=true;
	
	if( this._debugExs ){
		try{
			this._delegate.StartGame();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.StartGame();
	}
}

BBGame.prototype.SuspendGame=function(){

	if( !this._started || this._suspended ) return;
	this._suspended=true;
	
	if( this._debugExs ){
		try{
			this._delegate.SuspendGame();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.SuspendGame();
	}
}

BBGame.prototype.ResumeGame=function(){

	if( !this._started || !this._suspended ) return;
	this._suspended=false;
	
	if( this._debugExs ){
		try{
			this._delegate.ResumeGame();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.ResumeGame();
	}
}

BBGame.prototype.UpdateGame=function(){

	if( !this._started || this._suspended ) return;

	if( this._debugExs ){
		try{
			this._delegate.UpdateGame();
		}catch( ex ){
			this.Die( ex );
		}	
	}else{
		this._delegate.UpdateGame();
	}
}

BBGame.prototype.RenderGame=function(){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.RenderGame();
		}catch( ex ){
			this.Die( ex );
		}	
	}else{
		this._delegate.RenderGame();
	}
}

BBGame.prototype.KeyEvent=function( ev,data ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.KeyEvent( ev,data );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.KeyEvent( ev,data );
	}
}

BBGame.prototype.MouseEvent=function( ev,data,x,y ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.MouseEvent( ev,data,x,y );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.MouseEvent( ev,data,x,y );
	}
}

BBGame.prototype.TouchEvent=function( ev,data,x,y ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.TouchEvent( ev,data,x,y );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.TouchEvent( ev,data,x,y );
	}
}

BBGame.prototype.MotionEvent=function( ev,data,x,y,z ){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.MotionEvent( ev,data,x,y,z );
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.MotionEvent( ev,data,x,y,z );
	}
}

BBGame.prototype.DiscardGraphics=function(){

	if( !this._started ) return;
	
	if( this._debugExs ){
		try{
			this._delegate.DiscardGraphics();
		}catch( ex ){
			this.Die( ex );
		}
	}else{
		this._delegate.DiscardGraphics();
	}
}


var webglGraphicsSeq=1;

function BBHtml5Game( canvas ){

	BBGame.call( this );
	BBHtml5Game._game=this;
	this._canvas=canvas;
	this._loading=0;
	this._timerSeq=0;
	this._gl=null;
	
	if( CFG_OPENGL_GLES20_ENABLED=="1" ){

		//can't get these to fire!
		canvas.addEventListener( "webglcontextlost",function( event ){
			event.preventDefault();
//			print( "WebGL context lost!" );
		},false );

		canvas.addEventListener( "webglcontextrestored",function( event ){
			++webglGraphicsSeq;
//			print( "WebGL context restored!" );
		},false );

		var attrs={ alpha:false };
	
		this._gl=this._canvas.getContext( "webgl",attrs );

		if( !this._gl ) this._gl=this._canvas.getContext( "experimental-webgl",attrs );
		
		if( !this._gl ) this.Die( "Can't create WebGL" );
		
		gl=this._gl;
	}
}

BBHtml5Game.prototype=extend_class( BBGame );

BBHtml5Game.Html5Game=function(){
	return BBHtml5Game._game;
}

BBHtml5Game.prototype.ValidateUpdateTimer=function(){

	++this._timerSeq;
	if( this._suspended ) return;
	
	var game=this;
	var seq=game._timerSeq;
	
	var maxUpdates=4;
	var updateRate=this._updateRate;
	
	if( !updateRate ){

		var reqAnimFrame=(window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame);
	
		if( reqAnimFrame ){
			function animate(){
				if( seq!=game._timerSeq ) return;
	
				game.UpdateGame();
				if( seq!=game._timerSeq ) return;
	
				reqAnimFrame( animate );
				game.RenderGame();
			}
			reqAnimFrame( animate );
			return;
		}
		
		maxUpdates=1;
		updateRate=60;
	}
	
	var updatePeriod=1000.0/updateRate;
	var nextUpdate=0;

	function timeElapsed(){
		if( seq!=game._timerSeq ) return;
		
		if( !nextUpdate ) nextUpdate=Date.now();
		
		for( var i=0;i<maxUpdates;++i ){
		
			game.UpdateGame();
			if( seq!=game._timerSeq ) return;
			
			nextUpdate+=updatePeriod;
			var delay=nextUpdate-Date.now();
			
			if( delay>0 ){
				setTimeout( timeElapsed,delay );
				game.RenderGame();
				return;
			}
		}
		nextUpdate=0;
		setTimeout( timeElapsed,0 );
		game.RenderGame();
	}

	setTimeout( timeElapsed,0 );
}

//***** BBGame methods *****

BBHtml5Game.prototype.SetUpdateRate=function( updateRate ){

	BBGame.prototype.SetUpdateRate.call( this,updateRate );
	
	this.ValidateUpdateTimer();
}

BBHtml5Game.prototype.GetMetaData=function( path,key ){
	if( path.indexOf( "monkey://data/" )!=0 ) return "";
	path=path.slice(14);

	var i=META_DATA.indexOf( "["+path+"]" );
	if( i==-1 ) return "";
	i+=path.length+2;

	var e=META_DATA.indexOf( "\n",i );
	if( e==-1 ) e=META_DATA.length;

	i=META_DATA.indexOf( ";"+key+"=",i )
	if( i==-1 || i>=e ) return "";
	i+=key.length+2;

	e=META_DATA.indexOf( ";",i );
	if( e==-1 ) return "";

	return META_DATA.slice( i,e );
}

BBHtml5Game.prototype.PathToUrl=function( path ){
	if( path.indexOf( "monkey:" )!=0 ){
		return path;
	}else if( path.indexOf( "monkey://data/" )==0 ) {
		return "data/"+path.slice( 14 );
	}
	return "";
}

BBHtml5Game.prototype.GetLoading=function(){
	return this._loading;
}

BBHtml5Game.prototype.IncLoading=function(){
	++this._loading;
	return this._loading;
}

BBHtml5Game.prototype.DecLoading=function(){
	--this._loading;
	return this._loading;
}

BBHtml5Game.prototype.GetCanvas=function(){
	return this._canvas;
}

BBHtml5Game.prototype.GetWebGL=function(){
	return this._gl;
}

BBHtml5Game.prototype.GetDeviceWidth=function(){
	return this._canvas.width;
}

BBHtml5Game.prototype.GetDeviceHeight=function(){
	return this._canvas.height;
}

//***** INTERNAL *****

BBHtml5Game.prototype.UpdateGame=function(){

	if( !this._loading ) BBGame.prototype.UpdateGame.call( this );
}

BBHtml5Game.prototype.SuspendGame=function(){

	BBGame.prototype.SuspendGame.call( this );
	
	BBGame.prototype.RenderGame.call( this );
	
	this.ValidateUpdateTimer();
}

BBHtml5Game.prototype.ResumeGame=function(){

	BBGame.prototype.ResumeGame.call( this );
	
	this.ValidateUpdateTimer();
}

BBHtml5Game.prototype.Run=function(){

	var game=this;
	var canvas=game._canvas;
	
	var xscale=1;
	var yscale=1;
	
	var touchIds=new Array( 32 );
	for( i=0;i<32;++i ) touchIds[i]=-1;
	
	function eatEvent( e ){
		if( e.stopPropagation ){
			e.stopPropagation();
			e.preventDefault();
		}else{
			e.cancelBubble=true;
			e.returnValue=false;
		}
	}
	
	function keyToChar( key ){
		switch( key ){
		case 8:case 9:case 13:case 27:case 32:return key;
		case 33:case 34:case 35:case 36:case 37:case 38:case 39:case 40:case 45:return key|0x10000;
		case 46:return 127;
		}
		return 0;
	}
	
	function mouseX( e ){
		var x=e.clientX+document.body.scrollLeft;
		var c=canvas;
		while( c ){
			x-=c.offsetLeft;
			c=c.offsetParent;
		}
		return x*xscale;
	}
	
	function mouseY( e ){
		var y=e.clientY+document.body.scrollTop;
		var c=canvas;
		while( c ){
			y-=c.offsetTop;
			c=c.offsetParent;
		}
		return y*yscale;
	}

	function touchX( touch ){
		var x=touch.pageX;
		var c=canvas;
		while( c ){
			x-=c.offsetLeft;
			c=c.offsetParent;
		}
		return x;
	}			
	
	function touchY( touch ){
		var y=touch.pageY;
		var c=canvas;
		while( c ){
			y-=c.offsetTop;
			c=c.offsetParent;
		}
		return y;
	}
	
	canvas.onkeydown=function( e ){
		game.KeyEvent( BBGameEvent.KeyDown,e.keyCode );
		var chr=keyToChar( e.keyCode );
		if( chr ) game.KeyEvent( BBGameEvent.KeyChar,chr );
		if( e.keyCode<48 || (e.keyCode>111 && e.keyCode<122) ) eatEvent( e );
	}

	canvas.onkeyup=function( e ){
		game.KeyEvent( BBGameEvent.KeyUp,e.keyCode );
	}

	canvas.onkeypress=function( e ){
		if( e.charCode ){
			game.KeyEvent( BBGameEvent.KeyChar,e.charCode );
		}else if( e.which ){
			game.KeyEvent( BBGameEvent.KeyChar,e.which );
		}
	}

	canvas.onmousedown=function( e ){
		switch( e.button ){
		case 0:game.MouseEvent( BBGameEvent.MouseDown,0,mouseX(e),mouseY(e) );break;
		case 1:game.MouseEvent( BBGameEvent.MouseDown,2,mouseX(e),mouseY(e) );break;
		case 2:game.MouseEvent( BBGameEvent.MouseDown,1,mouseX(e),mouseY(e) );break;
		}
		eatEvent( e );
	}
	
	canvas.onmouseup=function( e ){
		switch( e.button ){
		case 0:game.MouseEvent( BBGameEvent.MouseUp,0,mouseX(e),mouseY(e) );break;
		case 1:game.MouseEvent( BBGameEvent.MouseUp,2,mouseX(e),mouseY(e) );break;
		case 2:game.MouseEvent( BBGameEvent.MouseUp,1,mouseX(e),mouseY(e) );break;
		}
		eatEvent( e );
	}
	
	canvas.onmousemove=function( e ){
		game.MouseEvent( BBGameEvent.MouseMove,-1,mouseX(e),mouseY(e) );
		eatEvent( e );
	}

	canvas.onmouseout=function( e ){
		game.MouseEvent( BBGameEvent.MouseUp,0,mouseX(e),mouseY(e) );
		game.MouseEvent( BBGameEvent.MouseUp,1,mouseX(e),mouseY(e) );
		game.MouseEvent( BBGameEvent.MouseUp,2,mouseX(e),mouseY(e) );
		eatEvent( e );
	}
	
	canvas.onclick=function( e ){
		if( game.Suspended() ){
			canvas.focus();
		}
		eatEvent( e );
		return;
	}
	
	canvas.oncontextmenu=function( e ){
		return false;
	}
	
	canvas.ontouchstart=function( e ){
		if( game.Suspended() ){
			canvas.focus();
		}
		for( var i=0;i<e.changedTouches.length;++i ){
			var touch=e.changedTouches[i];
			for( var j=0;j<32;++j ){
				if( touchIds[j]!=-1 ) continue;
				touchIds[j]=touch.identifier;
				game.TouchEvent( BBGameEvent.TouchDown,j,touchX(touch),touchY(touch) );
				break;
			}
		}
		eatEvent( e );
	}
	
	canvas.ontouchmove=function( e ){
		for( var i=0;i<e.changedTouches.length;++i ){
			var touch=e.changedTouches[i];
			for( var j=0;j<32;++j ){
				if( touchIds[j]!=touch.identifier ) continue;
				game.TouchEvent( BBGameEvent.TouchMove,j,touchX(touch),touchY(touch) );
				break;
			}
		}
		eatEvent( e );
	}
	
	canvas.ontouchend=function( e ){
		for( var i=0;i<e.changedTouches.length;++i ){
			var touch=e.changedTouches[i];
			for( var j=0;j<32;++j ){
				if( touchIds[j]!=touch.identifier ) continue;
				touchIds[j]=-1;
				game.TouchEvent( BBGameEvent.TouchUp,j,touchX(touch),touchY(touch) );
				break;
			}
		}
		eatEvent( e );
	}
	
	window.ondevicemotion=function( e ){
		var tx=e.accelerationIncludingGravity.x/9.81;
		var ty=e.accelerationIncludingGravity.y/9.81;
		var tz=e.accelerationIncludingGravity.z/9.81;
		var x,y;
		switch( window.orientation ){
		case   0:x=+tx;y=-ty;break;
		case 180:x=-tx;y=+ty;break;
		case  90:x=-ty;y=-tx;break;
		case -90:x=+ty;y=+tx;break;
		}
		game.MotionEvent( BBGameEvent.MotionAccel,0,x,y,tz );
		eatEvent( e );
	}

	canvas.onfocus=function( e ){
		if( CFG_MOJO_AUTO_SUSPEND_ENABLED=="1" ){
			game.ResumeGame();
		}
	}
	
	canvas.onblur=function( e ){
		for( var i=0;i<256;++i ) game.KeyEvent( BBGameEvent.KeyUp,i );
		if( CFG_MOJO_AUTO_SUSPEND_ENABLED=="1" ){
			game.SuspendGame();
		}
	}
	
	canvas.updateSize=function(){
		xscale=canvas.width/canvas.clientWidth;
		yscale=canvas.height/canvas.clientHeight;
		game.RenderGame();
	}
	
	canvas.updateSize();
	
	canvas.focus();
	
	game.StartGame();

	game.RenderGame();
}


function BBMonkeyGame( canvas ){
	BBHtml5Game.call( this,canvas );
}

BBMonkeyGame.prototype=extend_class( BBHtml5Game );

BBMonkeyGame.Main=function( canvas ){

	var game=new BBMonkeyGame( canvas );

	try{

		bbInit();
		bbMain();

	}catch( ex ){
	
		game.Die( ex );
		return;
	}

	if( !game.Delegate() ) return;
	
	game.Run();
}


// HTML5 mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.

// ***** gxtkGraphics class *****

function gxtkGraphics(){
	this.game=BBHtml5Game.Html5Game();
	this.canvas=this.game.GetCanvas()
	this.width=this.canvas.width;
	this.height=this.canvas.height;
	this.gl=null;
	this.gc=this.canvas.getContext( '2d' );
	this.tmpCanvas=null;
	this.r=255;
	this.b=255;
	this.g=255;
	this.white=true;
	this.color="rgb(255,255,255)"
	this.alpha=1;
	this.blend="source-over";
	this.ix=1;this.iy=0;
	this.jx=0;this.jy=1;
	this.tx=0;this.ty=0;
	this.tformed=false;
	this.scissorX=0;
	this.scissorY=0;
	this.scissorWidth=0;
	this.scissorHeight=0;
	this.clipped=false;
}

gxtkGraphics.prototype.BeginRender=function(){
	this.width=this.canvas.width;
	this.height=this.canvas.height;
	if( !this.gc ) return 0;
	this.gc.save();
	if( this.game.GetLoading() ) return 2;
	return 1;
}

gxtkGraphics.prototype.EndRender=function(){
	if( this.gc ) this.gc.restore();
}

gxtkGraphics.prototype.Width=function(){
	return this.width;
}

gxtkGraphics.prototype.Height=function(){
	return this.height;
}

gxtkGraphics.prototype.LoadSurface=function( path ){
	var game=this.game;

	var ty=game.GetMetaData( path,"type" );
	if( ty.indexOf( "image/" )!=0 ) return null;
	
	game.IncLoading();

	var image=new Image();
	image.onload=function(){ game.DecLoading(); }
	image.onerror=function(){ game.DecLoading(); }
	image.meta_width=parseInt( game.GetMetaData( path,"width" ) );
	image.meta_height=parseInt( game.GetMetaData( path,"height" ) );
	image.src=game.PathToUrl( path );

	return new gxtkSurface( image,this );
}

gxtkGraphics.prototype.CreateSurface=function( width,height ){
	var canvas=document.createElement( 'canvas' );
	
	canvas.width=width;
	canvas.height=height;
	canvas.meta_width=width;
	canvas.meta_height=height;
	canvas.complete=true;
	
	var surface=new gxtkSurface( canvas,this );
	
	surface.gc=canvas.getContext( '2d' );
	
	return surface;
}

gxtkGraphics.prototype.SetAlpha=function( alpha ){
	this.alpha=alpha;
	this.gc.globalAlpha=alpha;
}

gxtkGraphics.prototype.SetColor=function( r,g,b ){
	this.r=r;
	this.g=g;
	this.b=b;
	this.white=(r==255 && g==255 && b==255);
	this.color="rgb("+(r|0)+","+(g|0)+","+(b|0)+")";
	this.gc.fillStyle=this.color;
	this.gc.strokeStyle=this.color;
}

gxtkGraphics.prototype.SetBlend=function( blend ){
	switch( blend ){
	case 1:
		this.blend="lighter";
		break;
	default:
		this.blend="source-over";
	}
	this.gc.globalCompositeOperation=this.blend;
}

gxtkGraphics.prototype.SetScissor=function( x,y,w,h ){
	this.scissorX=x;
	this.scissorY=y;
	this.scissorWidth=w;
	this.scissorHeight=h;
	this.clipped=(x!=0 || y!=0 || w!=this.canvas.width || h!=this.canvas.height);
	this.gc.restore();
	this.gc.save();
	if( this.clipped ){
		this.gc.beginPath();
		this.gc.rect( x,y,w,h );
		this.gc.clip();
		this.gc.closePath();
	}
	this.gc.fillStyle=this.color;
	this.gc.strokeStyle=this.color;	
	this.gc.globalAlpha=this.alpha;	
	this.gc.globalCompositeOperation=this.blend;
	if( this.tformed ) this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
}

gxtkGraphics.prototype.SetMatrix=function( ix,iy,jx,jy,tx,ty ){
	this.ix=ix;this.iy=iy;
	this.jx=jx;this.jy=jy;
	this.tx=tx;this.ty=ty;
	this.gc.setTransform( ix,iy,jx,jy,tx,ty );
	this.tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);
}

gxtkGraphics.prototype.Cls=function( r,g,b ){
	if( this.tformed ) this.gc.setTransform( 1,0,0,1,0,0 );
	this.gc.fillStyle="rgb("+(r|0)+","+(g|0)+","+(b|0)+")";
	this.gc.globalAlpha=1;
	this.gc.globalCompositeOperation="source-over";
	this.gc.fillRect( 0,0,this.canvas.width,this.canvas.height );
	this.gc.fillStyle=this.color;
	this.gc.globalAlpha=this.alpha;
	this.gc.globalCompositeOperation=this.blend;
	if( this.tformed ) this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
}

gxtkGraphics.prototype.DrawPoint=function( x,y ){
	if( this.tformed ){
		var px=x;
		x=px * this.ix + y * this.jx + this.tx;
		y=px * this.iy + y * this.jy + this.ty;
		this.gc.setTransform( 1,0,0,1,0,0 );
		this.gc.fillRect( x,y,1,1 );
		this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
	}else{
		this.gc.fillRect( x,y,1,1 );
	}
}

gxtkGraphics.prototype.DrawRect=function( x,y,w,h ){
	if( w<0 ){ x+=w;w=-w; }
	if( h<0 ){ y+=h;h=-h; }
	if( w<=0 || h<=0 ) return;
	//
	this.gc.fillRect( x,y,w,h );
}

gxtkGraphics.prototype.DrawLine=function( x1,y1,x2,y2 ){
	if( this.tformed ){
		var x1_t=x1 * this.ix + y1 * this.jx + this.tx;
		var y1_t=x1 * this.iy + y1 * this.jy + this.ty;
		var x2_t=x2 * this.ix + y2 * this.jx + this.tx;
		var y2_t=x2 * this.iy + y2 * this.jy + this.ty;
		this.gc.setTransform( 1,0,0,1,0,0 );
	  	this.gc.beginPath();
	  	this.gc.moveTo( x1_t,y1_t );
	  	this.gc.lineTo( x2_t,y2_t );
	  	this.gc.stroke();
	  	this.gc.closePath();
		this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
	}else{
	  	this.gc.beginPath();
	  	this.gc.moveTo( x1,y1 );
	  	this.gc.lineTo( x2,y2 );
	  	this.gc.stroke();
	  	this.gc.closePath();
	}
}

gxtkGraphics.prototype.DrawOval=function( x,y,w,h ){
	if( w<0 ){ x+=w;w=-w; }
	if( h<0 ){ y+=h;h=-h; }
	if( w<=0 || h<=0 ) return;
	//
  	var w2=w/2,h2=h/2;
	this.gc.save();
	this.gc.translate( x+w2,y+h2 );
	this.gc.scale( w2,h2 );
  	this.gc.beginPath();
	this.gc.arc( 0,0,1,0,Math.PI*2,false );
	this.gc.fill();
  	this.gc.closePath();
	this.gc.restore();
}

gxtkGraphics.prototype.DrawPoly=function( verts ){
	if( verts.length<2 ) return;
	this.gc.beginPath();
	this.gc.moveTo( verts[0],verts[1] );
	for( var i=2;i<verts.length;i+=2 ){
		this.gc.lineTo( verts[i],verts[i+1] );
	}
	this.gc.fill();
	this.gc.closePath();
}

gxtkGraphics.prototype.DrawPoly2=function( verts,surface,srx,srcy ){
	if( verts.length<4 ) return;
	this.gc.beginPath();
	this.gc.moveTo( verts[0],verts[1] );
	for( var i=4;i<verts.length;i+=4 ){
		this.gc.lineTo( verts[i],verts[i+1] );
	}
	this.gc.fill();
	this.gc.closePath();
}

gxtkGraphics.prototype.DrawSurface=function( surface,x,y ){
	if( !surface.image.complete ) return;
	
	if( this.white ){
		this.gc.drawImage( surface.image,x,y );
		return;
	}
	
	this.DrawImageTinted( surface.image,x,y,0,0,surface.swidth,surface.sheight );
}

gxtkGraphics.prototype.DrawSurface2=function( surface,x,y,srcx,srcy,srcw,srch ){
	if( !surface.image.complete ) return;

	if( srcw<0 ){ srcx+=srcw;srcw=-srcw; }
	if( srch<0 ){ srcy+=srch;srch=-srch; }
	if( srcw<=0 || srch<=0 ) return;

	if( this.white ){
		this.gc.drawImage( surface.image,srcx,srcy,srcw,srch,x,y,srcw,srch );
		return;
	}
	
	this.DrawImageTinted( surface.image,x,y,srcx,srcy,srcw,srch  );
}

gxtkGraphics.prototype.DrawImageTinted=function( image,dx,dy,sx,sy,sw,sh ){

	if( !this.tmpCanvas ){
		this.tmpCanvas=document.createElement( "canvas" );
	}

	if( sw>this.tmpCanvas.width || sh>this.tmpCanvas.height ){
		this.tmpCanvas.width=Math.max( sw,this.tmpCanvas.width );
		this.tmpCanvas.height=Math.max( sh,this.tmpCanvas.height );
	}
	
	var tmpGC=this.tmpCanvas.getContext( "2d" );
	tmpGC.globalCompositeOperation="copy";
	
	tmpGC.drawImage( image,sx,sy,sw,sh,0,0,sw,sh );
	
	var imgData=tmpGC.getImageData( 0,0,sw,sh );
	
	var p=imgData.data,sz=sw*sh*4,i;
	
	for( i=0;i<sz;i+=4 ){
		p[i]=p[i]*this.r/255;
		p[i+1]=p[i+1]*this.g/255;
		p[i+2]=p[i+2]*this.b/255;
	}
	
	tmpGC.putImageData( imgData,0,0 );
	
	this.gc.drawImage( this.tmpCanvas,0,0,sw,sh,dx,dy,sw,sh );
}

gxtkGraphics.prototype.ReadPixels=function( pixels,x,y,width,height,offset,pitch ){

	var imgData=this.gc.getImageData( x,y,width,height );
	
	var p=imgData.data,i=0,j=offset,px,py;
	
	for( py=0;py<height;++py ){
		for( px=0;px<width;++px ){
			pixels[j++]=(p[i+3]<<24)|(p[i]<<16)|(p[i+1]<<8)|p[i+2];
			i+=4;
		}
		j+=pitch-width;
	}
}

gxtkGraphics.prototype.WritePixels2=function( surface,pixels,x,y,width,height,offset,pitch ){

	if( !surface.gc ){
		if( !surface.image.complete ) return;
		var canvas=document.createElement( "canvas" );
		canvas.width=surface.swidth;
		canvas.height=surface.sheight;
		surface.gc=canvas.getContext( "2d" );
		surface.gc.globalCompositeOperation="copy";
		surface.gc.drawImage( surface.image,0,0 );
		surface.image=canvas;
	}

	var imgData=surface.gc.createImageData( width,height );

	var p=imgData.data,i=0,j=offset,px,py,argb;
	
	for( py=0;py<height;++py ){
		for( px=0;px<width;++px ){
			argb=pixels[j++];
			p[i]=(argb>>16) & 0xff;
			p[i+1]=(argb>>8) & 0xff;
			p[i+2]=argb & 0xff;
			p[i+3]=(argb>>24) & 0xff;
			i+=4;
		}
		j+=pitch-width;
	}
	
	surface.gc.putImageData( imgData,x,y );
}

// ***** gxtkSurface class *****

function gxtkSurface( image,graphics ){
	this.image=image;
	this.graphics=graphics;
	this.swidth=image.meta_width;
	this.sheight=image.meta_height;
}

// ***** GXTK API *****

gxtkSurface.prototype.Discard=function(){
	if( this.image ){
		this.image=null;
	}
}

gxtkSurface.prototype.Width=function(){
	return this.swidth;
}

gxtkSurface.prototype.Height=function(){
	return this.sheight;
}

gxtkSurface.prototype.Loaded=function(){
	return this.image.complete;
}

gxtkSurface.prototype.OnUnsafeLoadComplete=function(){
}

if( CFG_HTML5_WEBAUDIO_ENABLED=="1" && (window.AudioContext || window.webkitAudioContext) ){

//print( "Using WebAudio!" );

// ***** WebAudio *****

var wa=null;

// ***** WebAudio gxtkSample *****

var gxtkSample=function(){
	this.waBuffer=null;
	this.state=0;
}

gxtkSample.prototype.Load=function( path ){
	if( this.state ) return false;

	var req=new XMLHttpRequest();
	
	req.open( "get",BBGame.Game().PathToUrl( path ),true );
	req.responseType="arraybuffer";
	
	var abuf=this;
	
	req.onload=function(){
		wa.decodeAudioData( req.response,function( buffer ){
			//success!
			abuf.waBuffer=buffer;
			abuf.state=1;
		},function(){
			abuf.state=-1;
		} );
	}
	
	req.onerror=function(){
		abuf.state=-1;
	}
	
	req.send();
	
	this.state=2;
			
	return true;
}

gxtkSample.prototype.Discard=function(){
}

// ***** WebAudio gxtkChannel *****

var gxtkChannel=function(){
	this.buffer=null;
	this.flags=0;
	this.volume=1;
	this.pan=0;
	this.rate=1;
	this.waSource=null;
	this.waPan=wa.create
	this.waGain=wa.createGain();
	this.waGain.connect( wa.destination );
	this.waPanner=wa.createPanner();
	this.waPanner.rolloffFactor=0;
	this.waPanner.panningModel="equalpower";
	this.waPanner.connect( this.waGain );
	this.startTime=0;
	this.offset=0;
	this.state=0;
}

// ***** WebAudio gxtkAudio *****

var gxtkAudio=function(){

	if( !wa ){
		window.AudioContext=window.AudioContext || window.webkitAudioContext;
		wa=new AudioContext();
	}
	
	this.okay=true;
	this.music=null;
	this.musicState=0;
	this.musicVolume=1;
	this.channels=new Array();
	for( var i=0;i<32;++i ){
		this.channels[i]=new gxtkChannel();
	}
}

gxtkAudio.prototype.Suspend=function(){
	if( this.MusicState()==1 ) this.music.pause();
	for( var i=0;i<32;++i ){
		var chan=this.channels[i];
		if( chan.state!=1 ) continue;
		this.PauseChannel( i );
		chan.state=5;
	}
}

gxtkAudio.prototype.Resume=function(){
	if( this.MusicState()==1 ) this.music.play();
	for( var i=0;i<32;++i ){
		var chan=this.channels[i];
		if( chan.state!=5 ) continue;
		chan.state=2;
		this.ResumeChannel( i );
	}
}

gxtkAudio.prototype.LoadSample=function( path ){

	var sample=new gxtkSample();
	if( !sample.Load( BBHtml5Game.Html5Game().PathToUrl( path ) ) ) return null;
	
	return sample;
}

gxtkAudio.prototype.PlaySample=function( buffer,channel,flags ){

	if( buffer.state!=1 ) return;

	var chan=this.channels[channel];
	
	if( chan.state ){
		chan.waSource.onended=null
		chan.waSource.stop( 0 );
	}
	
	chan.buffer=buffer;
	chan.flags=flags;

	chan.waSource=wa.createBufferSource();
	chan.waSource.buffer=buffer.waBuffer;
	chan.waSource.playbackRate.value=chan.rate;
	chan.waSource.loop=(flags&1)!=0;
	chan.waSource.connect( chan.waPanner );
	
	chan.waSource.onended=function( e ){
		chan.waSource=null;
		chan.state=0;
	}

	chan.offset=0;	
	chan.startTime=wa.currentTime;
	chan.waSource.start( 0 );

	chan.state=1;
}

gxtkAudio.prototype.StopChannel=function( channel ){

	var chan=this.channels[channel];
	if( !chan.state ) return;
	
	if( chan.state==1 ){
		chan.waSource.onended=null;
		chan.waSource.stop( 0 );
		chan.waSource=null;
	}

	chan.state=0;
}

gxtkAudio.prototype.PauseChannel=function( channel ){

	var chan=this.channels[channel];
	if( chan.state!=1 ) return;
	
	chan.offset=(chan.offset+(wa.currentTime-chan.startTime)*chan.rate)%chan.buffer.waBuffer.duration;
	
	chan.waSource.onended=null;
	chan.waSource.stop( 0 );
	chan.waSource=null;
	
	chan.state=2;
}

gxtkAudio.prototype.ResumeChannel=function( channel ){

	var chan=this.channels[channel];
	if( chan.state!=2 ) return;
	
	chan.waSource=wa.createBufferSource();
	chan.waSource.buffer=chan.buffer.waBuffer;
	chan.waSource.playbackRate.value=chan.rate;
	chan.waSource.loop=(chan.flags&1)!=0;
	chan.waSource.connect( chan.waPanner );
	
	chan.waSource.onended=function( e ){
		chan.waSource=null;
		chan.state=0;
	}
	
	chan.startTime=wa.currentTime;
	chan.waSource.start( 0,chan.offset );

	chan.state=1;
}

gxtkAudio.prototype.ChannelState=function( channel ){
	return this.channels[channel].state & 3;
}

gxtkAudio.prototype.SetVolume=function( channel,volume ){
	var chan=this.channels[channel];

	chan.volume=volume;
	
	chan.waGain.gain.value=volume;
}

gxtkAudio.prototype.SetPan=function( channel,pan ){
	var chan=this.channels[channel];

	chan.pan=pan;
	
	var sin=Math.sin( pan*3.14159265359/2 );
	var cos=Math.cos( pan*3.14159265359/2 );
	
	chan.waPanner.setPosition( sin,0,-cos );
}

gxtkAudio.prototype.SetRate=function( channel,rate ){

	var chan=this.channels[channel];

	if( chan.state==1 ){
		//update offset for pause/resume
		var time=wa.currentTime;
		chan.offset=(chan.offset+(time-chan.startTime)*chan.rate)%chan.buffer.waBuffer.duration;
		chan.startTime=time;
	}

	chan.rate=rate;
	
	if( chan.waSource ) chan.waSource.playbackRate.value=rate;
}

gxtkAudio.prototype.PlayMusic=function( path,flags ){
	if( this.musicState ) this.music.pause();
	this.music=new Audio( BBGame.Game().PathToUrl( path ) );
	this.music.loop=(flags&1)!=0;
	this.music.play();
	this.musicState=1;
}

gxtkAudio.prototype.StopMusic=function(){
	if( !this.musicState ) return;
	this.music.pause();
	this.music=null;
	this.musicState=0;
}

gxtkAudio.prototype.PauseMusic=function(){
	if( this.musicState!=1 ) return;
	this.music.pause();
	this.musicState=2;
}

gxtkAudio.prototype.ResumeMusic=function(){
	if( this.musicState!=2 ) return;
	this.music.play();
	this.musicState=1;
}

gxtkAudio.prototype.MusicState=function(){
	if( this.musicState==1 && this.music.ended && !this.music.loop ){
		this.music=null;
		this.musicState=0;
	}
	return this.musicState;
}

gxtkAudio.prototype.SetMusicVolume=function( volume ){
	this.musicVolume=volume;
	if( this.musicState ) this.music.volume=volume;
}

}else{

//print( "Using OldAudio!" );

// ***** gxtkChannel class *****

var gxtkChannel=function(){
	this.sample=null;
	this.audio=null;
	this.volume=1;
	this.pan=0;
	this.rate=1;
	this.flags=0;
	this.state=0;
}

// ***** gxtkAudio class *****

var gxtkAudio=function(){
	this.game=BBHtml5Game.Html5Game();
	this.okay=typeof(Audio)!="undefined";
	this.music=null;
	this.channels=new Array(33);
	for( var i=0;i<33;++i ){
		this.channels[i]=new gxtkChannel();
		if( !this.okay ) this.channels[i].state=-1;
	}
}

gxtkAudio.prototype.Suspend=function(){
	var i;
	for( i=0;i<33;++i ){
		var chan=this.channels[i];
		if( chan.state==1 ){
			if( chan.audio.ended && !chan.audio.loop ){
				chan.state=0;
			}else{
				chan.audio.pause();
				chan.state=3;
			}
		}
	}
}

gxtkAudio.prototype.Resume=function(){
	var i;
	for( i=0;i<33;++i ){
		var chan=this.channels[i];
		if( chan.state==3 ){
			chan.audio.play();
			chan.state=1;
		}
	}
}

gxtkAudio.prototype.LoadSample=function( path ){
	if( !this.okay ) return null;

	var audio=new Audio( this.game.PathToUrl( path ) );
	if( !audio ) return null;
	
	return new gxtkSample( audio );
}

gxtkAudio.prototype.PlaySample=function( sample,channel,flags ){
	if( !this.okay ) return;
	
	var chan=this.channels[channel];

	if( chan.state>0 ){
		chan.audio.pause();
		chan.state=0;
	}
	
	for( var i=0;i<33;++i ){
		var chan2=this.channels[i];
		if( chan2.state==1 && chan2.audio.ended && !chan2.audio.loop ) chan.state=0;
		if( chan2.state==0 && chan2.sample ){
			chan2.sample.FreeAudio( chan2.audio );
			chan2.sample=null;
			chan2.audio=null;
		}
	}

	var audio=sample.AllocAudio();
	if( !audio ) return;

	audio.loop=(flags&1)!=0;
	audio.volume=chan.volume;
	audio.play();

	chan.sample=sample;
	chan.audio=audio;
	chan.flags=flags;
	chan.state=1;
}

gxtkAudio.prototype.StopChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state>0 ){
		chan.audio.pause();
		chan.state=0;
	}
}

gxtkAudio.prototype.PauseChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state==1 ){
		if( chan.audio.ended && !chan.audio.loop ){
			chan.state=0;
		}else{
			chan.audio.pause();
			chan.state=2;
		}
	}
}

gxtkAudio.prototype.ResumeChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state==2 ){
		chan.audio.play();
		chan.state=1;
	}
}

gxtkAudio.prototype.ChannelState=function( channel ){
	var chan=this.channels[channel];
	if( chan.state==1 && chan.audio.ended && !chan.audio.loop ) chan.state=0;
	if( chan.state==3 ) return 1;
	return chan.state;
}

gxtkAudio.prototype.SetVolume=function( channel,volume ){
	var chan=this.channels[channel];
	if( chan.state>0 ) chan.audio.volume=volume;
	chan.volume=volume;
}

gxtkAudio.prototype.SetPan=function( channel,pan ){
	var chan=this.channels[channel];
	chan.pan=pan;
}

gxtkAudio.prototype.SetRate=function( channel,rate ){
	var chan=this.channels[channel];
	chan.rate=rate;
}

gxtkAudio.prototype.PlayMusic=function( path,flags ){
	this.StopMusic();
	
	this.music=this.LoadSample( path );
	if( !this.music ) return;
	
	this.PlaySample( this.music,32,flags );
}

gxtkAudio.prototype.StopMusic=function(){
	this.StopChannel( 32 );

	if( this.music ){
		this.music.Discard();
		this.music=null;
	}
}

gxtkAudio.prototype.PauseMusic=function(){
	this.PauseChannel( 32 );
}

gxtkAudio.prototype.ResumeMusic=function(){
	this.ResumeChannel( 32 );
}

gxtkAudio.prototype.MusicState=function(){
	return this.ChannelState( 32 );
}

gxtkAudio.prototype.SetMusicVolume=function( volume ){
	this.SetVolume( 32,volume );
}

// ***** gxtkSample class *****

//function gxtkSample( audio ){
var gxtkSample=function( audio ){
	this.audio=audio;
	this.free=new Array();
	this.insts=new Array();
}

gxtkSample.prototype.FreeAudio=function( audio ){
	this.free.push( audio );
}

gxtkSample.prototype.AllocAudio=function(){
	var audio;
	while( this.free.length ){
		audio=this.free.pop();
		try{
			audio.currentTime=0;
			return audio;
		}catch( ex ){
//			print( "AUDIO ERROR1!" );
		}
	}
	
	//Max out?
	if( this.insts.length==8 ) return null;
	
	audio=new Audio( this.audio.src );
	
	//yucky loop handler for firefox!
	//
	audio.addEventListener( 'ended',function(){
		if( this.loop ){
			try{
				this.currentTime=0;
				this.play();
			}catch( ex ){
//				print( "AUDIO ERROR2!" );
			}
		}
	},false );

	this.insts.push( audio );
	return audio;
}

gxtkSample.prototype.Discard=function(){
}

}


function BBThread(){
	this.result=null;
	this.running=false;
}

BBThread.prototype.Start=function(){
	this.result=null;
	this.running=true;
	this.Run__UNSAFE__();
}

BBThread.prototype.IsRunning=function(){
	return this.running;
}

BBThread.prototype.Result=function(){
	return this.result;
}

BBThread.prototype.Run__UNSAFE__=function(){
	this.running=false;
}


function BBAsyncImageLoaderThread(){
	this._running=false;
}

BBAsyncImageLoaderThread.prototype.Start=function(){

	var thread=this;

	thread._surface=null;
	thread._result=false;
	thread._running=true;

	var image=new Image();

	image.onload=function( e ){
		image.meta_width=image.width;
		image.meta_height=image.height;
		thread._surface=new gxtkSurface( image,thread._device )
		thread._result=true;
		thread._running=false;
	}
	
	image.onerror=function( e ){
		thread._running=false;
	}
	
	image.src=BBGame.Game().PathToUrl( thread._path );
}

BBAsyncImageLoaderThread.prototype.IsRunning=function(){
	return this._running;
}



function BBAsyncSoundLoaderThread(){
	this._running=false;
}
  
if( CFG_HTML5_WEBAUDIO_ENABLED=="1" && (window.AudioContext || window.webkitAudioContext) ){

BBAsyncSoundLoaderThread.prototype.Start=function(){

	this._sample=null;
	if( !this._device.okay ) return;
	
	var thread=this;
	
	thread._sample=null;
	thread._result=false;
	thread._running=true;

	var req=new XMLHttpRequest();
	req.open( "get",BBGame.Game().PathToUrl( this._path ),true );
	req.responseType="arraybuffer";
	
	req.onload=function(){
		//load success!
		wa.decodeAudioData( req.response,function( buffer ){
			//decode success!
			thread._sample=new gxtkSample();
			thread._sample.waBuffer=buffer;
			thread._sample.state=1;
			thread._result=true;
			thread._running=false;
		},function(){	
			//decode fail!
			thread._running=false;
		} );
	}
	
	req.onerror=function(){
		//load fail!
		thread._running=false;
	}
	
	req.send();
}
	
}else{
 
BBAsyncSoundLoaderThread.prototype.Start=function(){

	this._sample=null;
	if( !this._device.okay ) return;
	
	var audio=new Audio();
	if( !audio ) return;
	
	var thread=this;
	
	thread._sample=null;
	thread._result=false;
	thread._running=true;

	audio.src=BBGame.Game().PathToUrl( this._path );
	audio.preload='auto';	
	
	var success=function( e ){
		thread._sample=new gxtkSample( audio );
		thread._result=true;
		thread._running=false;
		audio.removeEventListener( 'canplaythrough',success,false );
		audio.removeEventListener( 'error',error,false );
	}
	
	var error=function( e ){
		thread._running=false;
		audio.removeEventListener( 'canplaythrough',success,false );
		audio.removeEventListener( 'error',error,false );
	}
	
	audio.addEventListener( 'canplaythrough',success,false );
	audio.addEventListener( 'error',error,false );
	
	//voodoo fix for Chrome!
	var timer=setInterval( function(){ if( !thread._running ) clearInterval( timer ); },200 );
	
	audio.load();
}

}
  
BBAsyncSoundLoaderThread.prototype.IsRunning=function(){
	return this._running;
}


function BBDataBuffer(){
	this.arrayBuffer=null;
	this.length=0;
}

BBDataBuffer.tbuf=new ArrayBuffer(4);
BBDataBuffer.tbytes=new Int8Array( BBDataBuffer.tbuf );
BBDataBuffer.tshorts=new Int16Array( BBDataBuffer.tbuf );
BBDataBuffer.tints=new Int32Array( BBDataBuffer.tbuf );
BBDataBuffer.tfloats=new Float32Array( BBDataBuffer.tbuf );

BBDataBuffer.prototype._Init=function( buffer ){
	this.arrayBuffer=buffer;
	this.length=buffer.byteLength;
	this.bytes=new Int8Array( buffer );	
	this.shorts=new Int16Array( buffer,0,this.length/2 );	
	this.ints=new Int32Array( buffer,0,this.length/4 );	
	this.floats=new Float32Array( buffer,0,this.length/4 );
}

BBDataBuffer.prototype._New=function( length ){
	if( this.arrayBuffer ) return false;
	
	var buf=new ArrayBuffer( length );
	if( !buf ) return false;
	
	this._Init( buf );
	return true;
}

BBDataBuffer.prototype._Load=function( path ){
	if( this.arrayBuffer ) return false;
	
	var buf=BBGame.Game().LoadData( path );
	if( !buf ) return false;
	
	this._Init( buf );
	return true;
}

BBDataBuffer.prototype._LoadAsync=function( path,thread ){

	var buf=this;
	
	var xhr=new XMLHttpRequest();
	xhr.open( "GET",BBGame.Game().PathToUrl( path ),true );
	xhr.responseType="arraybuffer";
	
	xhr.onload=function(e){
		if( this.status==200 || this.status==0 ){
			buf._Init( xhr.response );
			thread.result=buf;
		}
		thread.running=false;
	}
	
	xhr.onerror=function(e){
		thread.running=false;
	}
	
	xhr.send();
}


BBDataBuffer.prototype.GetArrayBuffer=function(){
	return this.arrayBuffer;
}

BBDataBuffer.prototype.Length=function(){
	return this.length;
}

BBDataBuffer.prototype.Discard=function(){
	if( this.arrayBuffer ){
		this.arrayBuffer=null;
		this.length=0;
	}
}

BBDataBuffer.prototype.PokeByte=function( addr,value ){
	this.bytes[addr]=value;
}

BBDataBuffer.prototype.PokeShort=function( addr,value ){
	if( addr&1 ){
		BBDataBuffer.tshorts[0]=value;
		this.bytes[addr]=BBDataBuffer.tbytes[0];
		this.bytes[addr+1]=BBDataBuffer.tbytes[1];
		return;
	}
	this.shorts[addr>>1]=value;
}

BBDataBuffer.prototype.PokeInt=function( addr,value ){
	if( addr&3 ){
		BBDataBuffer.tints[0]=value;
		this.bytes[addr]=BBDataBuffer.tbytes[0];
		this.bytes[addr+1]=BBDataBuffer.tbytes[1];
		this.bytes[addr+2]=BBDataBuffer.tbytes[2];
		this.bytes[addr+3]=BBDataBuffer.tbytes[3];
		return;
	}
	this.ints[addr>>2]=value;
}

BBDataBuffer.prototype.PokeFloat=function( addr,value ){
	if( addr&3 ){
		BBDataBuffer.tfloats[0]=value;
		this.bytes[addr]=BBDataBuffer.tbytes[0];
		this.bytes[addr+1]=BBDataBuffer.tbytes[1];
		this.bytes[addr+2]=BBDataBuffer.tbytes[2];
		this.bytes[addr+3]=BBDataBuffer.tbytes[3];
		return;
	}
	this.floats[addr>>2]=value;
}

BBDataBuffer.prototype.PeekByte=function( addr ){
	return this.bytes[addr];
}

BBDataBuffer.prototype.PeekShort=function( addr ){
	if( addr&1 ){
		BBDataBuffer.tbytes[0]=this.bytes[addr];
		BBDataBuffer.tbytes[1]=this.bytes[addr+1];
		return BBDataBuffer.tshorts[0];
	}
	return this.shorts[addr>>1];
}

BBDataBuffer.prototype.PeekInt=function( addr ){
	if( addr&3 ){
		BBDataBuffer.tbytes[0]=this.bytes[addr];
		BBDataBuffer.tbytes[1]=this.bytes[addr+1];
		BBDataBuffer.tbytes[2]=this.bytes[addr+2];
		BBDataBuffer.tbytes[3]=this.bytes[addr+3];
		return BBDataBuffer.tints[0];
	}
	return this.ints[addr>>2];
}

BBDataBuffer.prototype.PeekFloat=function( addr ){
	if( addr&3 ){
		BBDataBuffer.tbytes[0]=this.bytes[addr];
		BBDataBuffer.tbytes[1]=this.bytes[addr+1];
		BBDataBuffer.tbytes[2]=this.bytes[addr+2];
		BBDataBuffer.tbytes[3]=this.bytes[addr+3];
		return BBDataBuffer.tfloats[0];
	}
	return this.floats[addr>>2];
}


function BBStream(){
}

BBStream.prototype.Eof=function(){
	return 0;
}

BBStream.prototype.Close=function(){
}

BBStream.prototype.Length=function(){
	return 0;
}

BBStream.prototype.Position=function(){
	return 0;
}

BBStream.prototype.Seek=function( position ){
	return 0;
}

BBStream.prototype.Read=function( buffer,offset,count ){
	return 0;
}

BBStream.prototype.Write=function( buffer,offset,count ){
	return 0;
}

function c_App(){
	Object.call(this);
}
c_App.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<152>";
	if((bb_app__app)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<152>";
		error("App has already been created");
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<153>";
	bb_app__app=this;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<154>";
	bb_app__delegate=c_GameDelegate.m_new.call(new c_GameDelegate);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<155>";
	bb_app__game.SetDelegate(bb_app__delegate);
	pop_err();
	return this;
}
c_App.prototype.p_OnResize=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnCreate=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnSuspend=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnResume=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnUpdate=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnLoading=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnRender=function(){
	push_err();
	pop_err();
	return 0;
}
c_App.prototype.p_OnClose=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<177>";
	bb_app_EndApp();
	pop_err();
	return 0;
}
c_App.prototype.p_OnBack=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<181>";
	this.p_OnClose();
	pop_err();
	return 0;
}
function c_DungeonGen(){
	c_App.call(this);
	this.m_Room1=null;
	this.m_GameState="START";
}
c_DungeonGen.prototype=extend_class(c_App);
c_DungeonGen.m_new=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<11>";
	c_App.m_new.call(this);
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<11>";
	pop_err();
	return this;
}
c_DungeonGen.prototype.p_OnCreate=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<16>";
	this.m_Room1=c_RoomMap.m_new.call(new c_RoomMap);
	pop_err();
	return 0;
}
c_DungeonGen.prototype.p_OnUpdate=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<21>";
	var t_1=this.m_GameState;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<22>";
	if(t_1=="START"){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<23>";
		if((bb_input_KeyHit(32))!=0){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<24>";
			bb_random_Seed=bb_app_Millisecs();
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<25>";
			this.m_Room1.p_Build();
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<26>";
			this.m_GameState="PLAYING";
		}
	}else{
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<28>";
		if(t_1=="PLAYING"){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<29>";
			if((bb_input_KeyHit(27))!=0){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<30>";
				this.m_Room1.p_Reset();
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<31>";
				this.m_GameState="START";
			}
		}
	}
	pop_err();
	return 0;
}
c_DungeonGen.prototype.p_OnRender=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<37>";
	var t_2=this.m_GameState;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<38>";
	if(t_2=="START"){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<39>";
		bb_graphics_Cls(0.0,0.0,0.0);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<40>";
		bb_graphics_SetColor(255.0,255.0,255.0);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<41>";
		bb_graphics_DrawText("Press Space to Start",255.0,255.0,0.0,0.0);
	}else{
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<42>";
		if(t_2=="PLAYING"){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<43>";
			bb_graphics_Cls(0.0,0.0,0.0);
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<44>";
			this.m_Room1.p_Draw();
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<45>";
			bb_graphics_SetColor(255.0,255.0,255.0);
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<46>";
			bb_graphics_DrawText("Press ESC to reset",255.0,255.0,0.0,0.0);
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<47>";
			bb_graphics_DrawText("Seed:",255.0,300.0,0.0,0.0);
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<48>";
			bb_graphics_DrawText(String(bb_random_Seed),300.0,300.0,0.0,0.0);
		}
	}
	pop_err();
	return 0;
}
var bb_app__app=null;
function c_GameDelegate(){
	BBGameDelegate.call(this);
	this.m__graphics=null;
	this.m__audio=null;
	this.m__input=null;
}
c_GameDelegate.prototype=extend_class(BBGameDelegate);
c_GameDelegate.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<65>";
	pop_err();
	return this;
}
c_GameDelegate.prototype.StartGame=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<75>";
	this.m__graphics=(new gxtkGraphics);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<76>";
	bb_graphics_SetGraphicsDevice(this.m__graphics);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<77>";
	bb_graphics_SetFont(null,32);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<79>";
	this.m__audio=(new gxtkAudio);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<80>";
	bb_audio_SetAudioDevice(this.m__audio);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<82>";
	this.m__input=c_InputDevice.m_new.call(new c_InputDevice);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<83>";
	bb_input_SetInputDevice(this.m__input);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<85>";
	bb_app_ValidateDeviceWindow(false);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<87>";
	bb_app_EnumDisplayModes();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<89>";
	bb_app__app.p_OnCreate();
	pop_err();
}
c_GameDelegate.prototype.SuspendGame=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<93>";
	bb_app__app.p_OnSuspend();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<94>";
	this.m__audio.Suspend();
	pop_err();
}
c_GameDelegate.prototype.ResumeGame=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<98>";
	this.m__audio.Resume();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<99>";
	bb_app__app.p_OnResume();
	pop_err();
}
c_GameDelegate.prototype.UpdateGame=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<103>";
	bb_app_ValidateDeviceWindow(true);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<104>";
	this.m__input.p_BeginUpdate();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<105>";
	bb_app__app.p_OnUpdate();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<106>";
	this.m__input.p_EndUpdate();
	pop_err();
}
c_GameDelegate.prototype.RenderGame=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<110>";
	bb_app_ValidateDeviceWindow(true);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<111>";
	var t_mode=this.m__graphics.BeginRender();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<112>";
	if((t_mode)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<112>";
		bb_graphics_BeginRender();
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<113>";
	if(t_mode==2){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<113>";
		bb_app__app.p_OnLoading();
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<113>";
		bb_app__app.p_OnRender();
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<114>";
	if((t_mode)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<114>";
		bb_graphics_EndRender();
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<115>";
	this.m__graphics.EndRender();
	pop_err();
}
c_GameDelegate.prototype.KeyEvent=function(t_event,t_data){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<119>";
	this.m__input.p_KeyEvent(t_event,t_data);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<120>";
	if(t_event!=1){
		pop_err();
		return;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<121>";
	var t_1=t_data;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<122>";
	if(t_1==432){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<123>";
		bb_app__app.p_OnClose();
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<124>";
		if(t_1==416){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<125>";
			bb_app__app.p_OnBack();
		}
	}
	pop_err();
}
c_GameDelegate.prototype.MouseEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<130>";
	this.m__input.p_MouseEvent(t_event,t_data,t_x,t_y);
	pop_err();
}
c_GameDelegate.prototype.TouchEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<134>";
	this.m__input.p_TouchEvent(t_event,t_data,t_x,t_y);
	pop_err();
}
c_GameDelegate.prototype.MotionEvent=function(t_event,t_data,t_x,t_y,t_z){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<138>";
	this.m__input.p_MotionEvent(t_event,t_data,t_x,t_y,t_z);
	pop_err();
}
c_GameDelegate.prototype.DiscardGraphics=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<142>";
	this.m__graphics.DiscardGraphics();
	pop_err();
}
var bb_app__delegate=null;
var bb_app__game=null;
var bb_LayeredLevel_Prototype=null;
function bbMain(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<6>";
	bb_LayeredLevel_Prototype=c_DungeonGen.m_new.call(new c_DungeonGen);
	pop_err();
	return 0;
}
var bb_graphics_device=null;
function bb_graphics_SetGraphicsDevice(t_dev){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<63>";
	bb_graphics_device=t_dev;
	pop_err();
	return 0;
}
function c_Image(){
	Object.call(this);
	this.m_surface=null;
	this.m_width=0;
	this.m_height=0;
	this.m_frames=[];
	this.m_flags=0;
	this.m_tx=.0;
	this.m_ty=.0;
	this.m_source=null;
}
c_Image.m_DefaultFlags=0;
c_Image.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<70>";
	pop_err();
	return this;
}
c_Image.prototype.p_SetHandle=function(t_tx,t_ty){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<114>";
	dbg_object(this).m_tx=t_tx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<115>";
	dbg_object(this).m_ty=t_ty;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<116>";
	dbg_object(this).m_flags=dbg_object(this).m_flags&-2;
	pop_err();
	return 0;
}
c_Image.prototype.p_ApplyFlags=function(t_iflags){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<197>";
	this.m_flags=t_iflags;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<199>";
	if((this.m_flags&2)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<200>";
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<200>";
		var t_=this.m_frames;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<200>";
		var t_2=0;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<200>";
		while(t_2<t_.length){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<200>";
			var t_f=dbg_array(t_,t_2)[dbg_index];
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<200>";
			t_2=t_2+1;
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<201>";
			dbg_object(t_f).m_x+=1;
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<203>";
		this.m_width-=2;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<206>";
	if((this.m_flags&4)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<207>";
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<207>";
		var t_3=this.m_frames;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<207>";
		var t_4=0;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<207>";
		while(t_4<t_3.length){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<207>";
			var t_f2=dbg_array(t_3,t_4)[dbg_index];
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<207>";
			t_4=t_4+1;
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<208>";
			dbg_object(t_f2).m_y+=1;
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<210>";
		this.m_height-=2;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<213>";
	if((this.m_flags&1)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<214>";
		this.p_SetHandle((this.m_width)/2.0,(this.m_height)/2.0);
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<217>";
	if(this.m_frames.length==1 && dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_x==0 && dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_y==0 && this.m_width==this.m_surface.Width() && this.m_height==this.m_surface.Height()){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<218>";
		this.m_flags|=65536;
	}
	pop_err();
	return 0;
}
c_Image.prototype.p_Init=function(t_surf,t_nframes,t_iflags){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<143>";
	if((this.m_surface)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<143>";
		error("Image already initialized");
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<144>";
	this.m_surface=t_surf;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<146>";
	this.m_width=((this.m_surface.Width()/t_nframes)|0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<147>";
	this.m_height=this.m_surface.Height();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<149>";
	this.m_frames=new_object_array(t_nframes);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<150>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<151>";
		dbg_array(this.m_frames,t_i)[dbg_index]=c_Frame.m_new.call(new c_Frame,t_i*this.m_width,0);
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<154>";
	this.p_ApplyFlags(t_iflags);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<155>";
	pop_err();
	return this;
}
c_Image.prototype.p_Init2=function(t_surf,t_x,t_y,t_iwidth,t_iheight,t_nframes,t_iflags,t_src,t_srcx,t_srcy,t_srcw,t_srch){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<159>";
	if((this.m_surface)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<159>";
		error("Image already initialized");
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<160>";
	this.m_surface=t_surf;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<161>";
	this.m_source=t_src;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<163>";
	this.m_width=t_iwidth;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<164>";
	this.m_height=t_iheight;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<166>";
	this.m_frames=new_object_array(t_nframes);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<168>";
	var t_ix=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<168>";
	var t_iy=t_y;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<170>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<171>";
		if(t_ix+this.m_width>t_srcw){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<172>";
			t_ix=0;
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<173>";
			t_iy+=this.m_height;
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<175>";
		if(t_ix+this.m_width>t_srcw || t_iy+this.m_height>t_srch){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<176>";
			error("Image frame outside surface");
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<178>";
		dbg_array(this.m_frames,t_i)[dbg_index]=c_Frame.m_new.call(new c_Frame,t_ix+t_srcx,t_iy+t_srcy);
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<179>";
		t_ix+=this.m_width;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<182>";
	this.p_ApplyFlags(t_iflags);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<183>";
	pop_err();
	return this;
}
c_Image.prototype.p_Width=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<81>";
	pop_err();
	return this.m_width;
}
c_Image.prototype.p_Height=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<85>";
	pop_err();
	return this.m_height;
}
c_Image.prototype.p_Frames=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<93>";
	var t_=this.m_frames.length;
	pop_err();
	return t_;
}
c_Image.prototype.p_GrabImage=function(t_x,t_y,t_width,t_height,t_nframes,t_flags){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<109>";
	if(this.m_frames.length!=1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<109>";
		pop_err();
		return null;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<110>";
	var t_=(c_Image.m_new.call(new c_Image)).p_Init2(this.m_surface,t_x,t_y,t_width,t_height,t_nframes,t_flags,this,dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_x,dbg_object(dbg_array(this.m_frames,0)[dbg_index]).m_y,dbg_object(this).m_width,dbg_object(this).m_height);
	pop_err();
	return t_;
}
function c_GraphicsContext(){
	Object.call(this);
	this.m_defaultFont=null;
	this.m_font=null;
	this.m_firstChar=0;
	this.m_matrixSp=0;
	this.m_ix=1.0;
	this.m_iy=.0;
	this.m_jx=.0;
	this.m_jy=1.0;
	this.m_tx=.0;
	this.m_ty=.0;
	this.m_tformed=0;
	this.m_matDirty=0;
	this.m_color_r=.0;
	this.m_color_g=.0;
	this.m_color_b=.0;
	this.m_alpha=.0;
	this.m_blend=0;
	this.m_scissor_x=.0;
	this.m_scissor_y=.0;
	this.m_scissor_width=.0;
	this.m_scissor_height=.0;
	this.m_matrixStack=new_number_array(192);
}
c_GraphicsContext.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<29>";
	pop_err();
	return this;
}
c_GraphicsContext.prototype.p_Validate=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<40>";
	if((this.m_matDirty)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<41>";
		bb_graphics_renderDevice.SetMatrix(dbg_object(bb_graphics_context).m_ix,dbg_object(bb_graphics_context).m_iy,dbg_object(bb_graphics_context).m_jx,dbg_object(bb_graphics_context).m_jy,dbg_object(bb_graphics_context).m_tx,dbg_object(bb_graphics_context).m_ty);
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<42>";
		this.m_matDirty=0;
	}
	pop_err();
	return 0;
}
var bb_graphics_context=null;
function bb_data_FixDataPath(t_path){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/data.monkey<7>";
	var t_i=t_path.indexOf(":/",0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/data.monkey<8>";
	if(t_i!=-1 && t_path.indexOf("/",0)==t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/data.monkey<8>";
		pop_err();
		return t_path;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/data.monkey<9>";
	if(string_startswith(t_path,"./") || string_startswith(t_path,"/")){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/data.monkey<9>";
		pop_err();
		return t_path;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/data.monkey<10>";
	var t_="monkey://data/"+t_path;
	pop_err();
	return t_;
}
function c_Frame(){
	Object.call(this);
	this.m_x=0;
	this.m_y=0;
}
c_Frame.m_new=function(t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<23>";
	dbg_object(this).m_x=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<24>";
	dbg_object(this).m_y=t_y;
	pop_err();
	return this;
}
c_Frame.m_new2=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<18>";
	pop_err();
	return this;
}
function bb_graphics_LoadImage(t_path,t_frameCount,t_flags){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<239>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<240>";
	if((t_surf)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<240>";
		var t_=(c_Image.m_new.call(new c_Image)).p_Init(t_surf,t_frameCount,t_flags);
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<244>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<245>";
	if((t_surf)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<245>";
		var t_=(c_Image.m_new.call(new c_Image)).p_Init2(t_surf,0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags,null,0,0,t_surf.Width(),t_surf.Height());
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_SetFont(t_font,t_firstChar){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<548>";
	if(!((t_font)!=null)){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<549>";
		if(!((dbg_object(bb_graphics_context).m_defaultFont)!=null)){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<550>";
			dbg_object(bb_graphics_context).m_defaultFont=bb_graphics_LoadImage("mojo_font.png",96,2);
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<552>";
		t_font=dbg_object(bb_graphics_context).m_defaultFont;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<553>";
		t_firstChar=32;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<555>";
	dbg_object(bb_graphics_context).m_font=t_font;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<556>";
	dbg_object(bb_graphics_context).m_firstChar=t_firstChar;
	pop_err();
	return 0;
}
var bb_audio_device=null;
function bb_audio_SetAudioDevice(t_dev){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/audio.monkey<22>";
	bb_audio_device=t_dev;
	pop_err();
	return 0;
}
function c_InputDevice(){
	Object.call(this);
	this.m__joyStates=new_object_array(4);
	this.m__keyDown=new_bool_array(512);
	this.m__keyHitPut=0;
	this.m__keyHitQueue=new_number_array(33);
	this.m__keyHit=new_number_array(512);
	this.m__charGet=0;
	this.m__charPut=0;
	this.m__charQueue=new_number_array(32);
	this.m__mouseX=.0;
	this.m__mouseY=.0;
	this.m__touchX=new_number_array(32);
	this.m__touchY=new_number_array(32);
	this.m__accelX=.0;
	this.m__accelY=.0;
	this.m__accelZ=.0;
}
c_InputDevice.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<26>";
	for(var t_i=0;t_i<4;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<27>";
		dbg_array(this.m__joyStates,t_i)[dbg_index]=c_JoyState.m_new.call(new c_JoyState);
	}
	pop_err();
	return this;
}
c_InputDevice.prototype.p_PutKeyHit=function(t_key){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<237>";
	if(this.m__keyHitPut==this.m__keyHitQueue.length){
		pop_err();
		return;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<238>";
	dbg_array(this.m__keyHit,t_key)[dbg_index]+=1;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<239>";
	dbg_array(this.m__keyHitQueue,this.m__keyHitPut)[dbg_index]=t_key;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<240>";
	this.m__keyHitPut+=1;
	pop_err();
}
c_InputDevice.prototype.p_BeginUpdate=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<189>";
	for(var t_i=0;t_i<4;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<190>";
		var t_state=dbg_array(this.m__joyStates,t_i)[dbg_index];
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<191>";
		if(!BBGame.Game().PollJoystick(t_i,dbg_object(t_state).m_joyx,dbg_object(t_state).m_joyy,dbg_object(t_state).m_joyz,dbg_object(t_state).m_buttons)){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<191>";
			break;
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<192>";
		for(var t_j=0;t_j<32;t_j=t_j+1){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<193>";
			var t_key=256+t_i*32+t_j;
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<194>";
			if(dbg_array(dbg_object(t_state).m_buttons,t_j)[dbg_index]){
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<195>";
				if(!dbg_array(this.m__keyDown,t_key)[dbg_index]){
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<196>";
					dbg_array(this.m__keyDown,t_key)[dbg_index]=true;
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<197>";
					this.p_PutKeyHit(t_key);
				}
			}else{
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<200>";
				dbg_array(this.m__keyDown,t_key)[dbg_index]=false;
			}
		}
	}
	pop_err();
}
c_InputDevice.prototype.p_EndUpdate=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<207>";
	for(var t_i=0;t_i<this.m__keyHitPut;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<208>";
		dbg_array(this.m__keyHit,dbg_array(this.m__keyHitQueue,t_i)[dbg_index])[dbg_index]=0;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<210>";
	this.m__keyHitPut=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<211>";
	this.m__charGet=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<212>";
	this.m__charPut=0;
	pop_err();
}
c_InputDevice.prototype.p_KeyEvent=function(t_event,t_data){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<111>";
	var t_1=t_event;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<112>";
	if(t_1==1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<113>";
		if(!dbg_array(this.m__keyDown,t_data)[dbg_index]){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<114>";
			dbg_array(this.m__keyDown,t_data)[dbg_index]=true;
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<115>";
			this.p_PutKeyHit(t_data);
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<116>";
			if(t_data==1){
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<117>";
				dbg_array(this.m__keyDown,384)[dbg_index]=true;
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<118>";
				this.p_PutKeyHit(384);
			}else{
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<119>";
				if(t_data==384){
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<120>";
					dbg_array(this.m__keyDown,1)[dbg_index]=true;
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<121>";
					this.p_PutKeyHit(1);
				}
			}
		}
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<124>";
		if(t_1==2){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<125>";
			if(dbg_array(this.m__keyDown,t_data)[dbg_index]){
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<126>";
				dbg_array(this.m__keyDown,t_data)[dbg_index]=false;
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<127>";
				if(t_data==1){
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<128>";
					dbg_array(this.m__keyDown,384)[dbg_index]=false;
				}else{
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<129>";
					if(t_data==384){
						err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<130>";
						dbg_array(this.m__keyDown,1)[dbg_index]=false;
					}
				}
			}
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<133>";
			if(t_1==3){
				err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<134>";
				if(this.m__charPut<this.m__charQueue.length){
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<135>";
					dbg_array(this.m__charQueue,this.m__charPut)[dbg_index]=t_data;
					err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<136>";
					this.m__charPut+=1;
				}
			}
		}
	}
	pop_err();
}
c_InputDevice.prototype.p_MouseEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<142>";
	var t_2=t_event;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<143>";
	if(t_2==4){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<144>";
		this.p_KeyEvent(1,1+t_data);
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<145>";
		if(t_2==5){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<146>";
			this.p_KeyEvent(2,1+t_data);
			pop_err();
			return;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<148>";
			if(t_2==6){
			}else{
				pop_err();
				return;
			}
		}
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<152>";
	this.m__mouseX=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<153>";
	this.m__mouseY=t_y;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<154>";
	dbg_array(this.m__touchX,0)[dbg_index]=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<155>";
	dbg_array(this.m__touchY,0)[dbg_index]=t_y;
	pop_err();
}
c_InputDevice.prototype.p_TouchEvent=function(t_event,t_data,t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<159>";
	var t_3=t_event;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<160>";
	if(t_3==7){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<161>";
		this.p_KeyEvent(1,384+t_data);
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<162>";
		if(t_3==8){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<163>";
			this.p_KeyEvent(2,384+t_data);
			pop_err();
			return;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<165>";
			if(t_3==9){
			}else{
				pop_err();
				return;
			}
		}
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<169>";
	dbg_array(this.m__touchX,t_data)[dbg_index]=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<170>";
	dbg_array(this.m__touchY,t_data)[dbg_index]=t_y;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<171>";
	if(t_data==0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<172>";
		this.m__mouseX=t_x;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<173>";
		this.m__mouseY=t_y;
	}
	pop_err();
}
c_InputDevice.prototype.p_MotionEvent=function(t_event,t_data,t_x,t_y,t_z){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<178>";
	var t_4=t_event;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<179>";
	if(t_4==10){
	}else{
		pop_err();
		return;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<183>";
	this.m__accelX=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<184>";
	this.m__accelY=t_y;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<185>";
	this.m__accelZ=t_z;
	pop_err();
}
c_InputDevice.prototype.p_KeyHit=function(t_key){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<52>";
	if(t_key>0 && t_key<512){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<52>";
		pop_err();
		return dbg_array(this.m__keyHit,t_key)[dbg_index];
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<53>";
	pop_err();
	return 0;
}
function c_JoyState(){
	Object.call(this);
	this.m_joyx=new_number_array(2);
	this.m_joyy=new_number_array(2);
	this.m_joyz=new_number_array(2);
	this.m_buttons=new_bool_array(32);
}
c_JoyState.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/inputdevice.monkey<14>";
	pop_err();
	return this;
}
var bb_input_device=null;
function bb_input_SetInputDevice(t_dev){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/input.monkey<22>";
	bb_input_device=t_dev;
	pop_err();
	return 0;
}
var bb_app__devWidth=0;
var bb_app__devHeight=0;
function bb_app_ValidateDeviceWindow(t_notifyApp){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<57>";
	var t_w=bb_app__game.GetDeviceWidth();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<58>";
	var t_h=bb_app__game.GetDeviceHeight();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<59>";
	if(t_w==bb_app__devWidth && t_h==bb_app__devHeight){
		pop_err();
		return;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<60>";
	bb_app__devWidth=t_w;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<61>";
	bb_app__devHeight=t_h;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<62>";
	if(t_notifyApp){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<62>";
		bb_app__app.p_OnResize();
	}
	pop_err();
}
function c_DisplayMode(){
	Object.call(this);
	this.m__width=0;
	this.m__height=0;
}
c_DisplayMode.m_new=function(t_width,t_height){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<192>";
	this.m__width=t_width;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<193>";
	this.m__height=t_height;
	pop_err();
	return this;
}
c_DisplayMode.m_new2=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<189>";
	pop_err();
	return this;
}
function c_Map(){
	Object.call(this);
	this.m_root=null;
}
c_Map.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<7>";
	pop_err();
	return this;
}
c_Map.prototype.p_Compare=function(t_lhs,t_rhs){
}
c_Map.prototype.p_FindNode=function(t_key){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<157>";
	var t_node=this.m_root;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<159>";
	while((t_node)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<160>";
		var t_cmp=this.p_Compare(t_key,dbg_object(t_node).m_key);
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<161>";
		if(t_cmp>0){
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<162>";
			t_node=dbg_object(t_node).m_right;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<163>";
			if(t_cmp<0){
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<164>";
				t_node=dbg_object(t_node).m_left;
			}else{
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<166>";
				pop_err();
				return t_node;
			}
		}
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<169>";
	pop_err();
	return t_node;
}
c_Map.prototype.p_Contains=function(t_key){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<25>";
	var t_=this.p_FindNode(t_key)!=null;
	pop_err();
	return t_;
}
c_Map.prototype.p_RotateLeft=function(t_node){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<251>";
	var t_child=dbg_object(t_node).m_right;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<252>";
	dbg_object(t_node).m_right=dbg_object(t_child).m_left;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<253>";
	if((dbg_object(t_child).m_left)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<254>";
		dbg_object(dbg_object(t_child).m_left).m_parent=t_node;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<256>";
	dbg_object(t_child).m_parent=dbg_object(t_node).m_parent;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<257>";
	if((dbg_object(t_node).m_parent)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<258>";
		if(t_node==dbg_object(dbg_object(t_node).m_parent).m_left){
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<259>";
			dbg_object(dbg_object(t_node).m_parent).m_left=t_child;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<261>";
			dbg_object(dbg_object(t_node).m_parent).m_right=t_child;
		}
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<264>";
		this.m_root=t_child;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<266>";
	dbg_object(t_child).m_left=t_node;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<267>";
	dbg_object(t_node).m_parent=t_child;
	pop_err();
	return 0;
}
c_Map.prototype.p_RotateRight=function(t_node){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<271>";
	var t_child=dbg_object(t_node).m_left;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<272>";
	dbg_object(t_node).m_left=dbg_object(t_child).m_right;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<273>";
	if((dbg_object(t_child).m_right)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<274>";
		dbg_object(dbg_object(t_child).m_right).m_parent=t_node;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<276>";
	dbg_object(t_child).m_parent=dbg_object(t_node).m_parent;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<277>";
	if((dbg_object(t_node).m_parent)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<278>";
		if(t_node==dbg_object(dbg_object(t_node).m_parent).m_right){
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<279>";
			dbg_object(dbg_object(t_node).m_parent).m_right=t_child;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<281>";
			dbg_object(dbg_object(t_node).m_parent).m_left=t_child;
		}
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<284>";
		this.m_root=t_child;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<286>";
	dbg_object(t_child).m_right=t_node;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<287>";
	dbg_object(t_node).m_parent=t_child;
	pop_err();
	return 0;
}
c_Map.prototype.p_InsertFixup=function(t_node){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<212>";
	while(((dbg_object(t_node).m_parent)!=null) && dbg_object(dbg_object(t_node).m_parent).m_color==-1 && ((dbg_object(dbg_object(t_node).m_parent).m_parent)!=null)){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<213>";
		if(dbg_object(t_node).m_parent==dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_left){
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<214>";
			var t_uncle=dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_right;
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<215>";
			if(((t_uncle)!=null) && dbg_object(t_uncle).m_color==-1){
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<216>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<217>";
				dbg_object(t_uncle).m_color=1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<218>";
				dbg_object(dbg_object(t_uncle).m_parent).m_color=-1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<219>";
				t_node=dbg_object(t_uncle).m_parent;
			}else{
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<221>";
				if(t_node==dbg_object(dbg_object(t_node).m_parent).m_right){
					err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<222>";
					t_node=dbg_object(t_node).m_parent;
					err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<223>";
					this.p_RotateLeft(t_node);
				}
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<225>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<226>";
				dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_color=-1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<227>";
				this.p_RotateRight(dbg_object(dbg_object(t_node).m_parent).m_parent);
			}
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<230>";
			var t_uncle2=dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_left;
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<231>";
			if(((t_uncle2)!=null) && dbg_object(t_uncle2).m_color==-1){
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<232>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<233>";
				dbg_object(t_uncle2).m_color=1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<234>";
				dbg_object(dbg_object(t_uncle2).m_parent).m_color=-1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<235>";
				t_node=dbg_object(t_uncle2).m_parent;
			}else{
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<237>";
				if(t_node==dbg_object(dbg_object(t_node).m_parent).m_left){
					err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<238>";
					t_node=dbg_object(t_node).m_parent;
					err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<239>";
					this.p_RotateRight(t_node);
				}
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<241>";
				dbg_object(dbg_object(t_node).m_parent).m_color=1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<242>";
				dbg_object(dbg_object(dbg_object(t_node).m_parent).m_parent).m_color=-1;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<243>";
				this.p_RotateLeft(dbg_object(dbg_object(t_node).m_parent).m_parent);
			}
		}
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<247>";
	dbg_object(this.m_root).m_color=1;
	pop_err();
	return 0;
}
c_Map.prototype.p_Set=function(t_key,t_value){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<29>";
	var t_node=this.m_root;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<30>";
	var t_parent=null;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<30>";
	var t_cmp=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<32>";
	while((t_node)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<33>";
		t_parent=t_node;
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<34>";
		t_cmp=this.p_Compare(t_key,dbg_object(t_node).m_key);
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<35>";
		if(t_cmp>0){
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<36>";
			t_node=dbg_object(t_node).m_right;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<37>";
			if(t_cmp<0){
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<38>";
				t_node=dbg_object(t_node).m_left;
			}else{
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<40>";
				dbg_object(t_node).m_value=t_value;
				err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<41>";
				pop_err();
				return false;
			}
		}
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<45>";
	t_node=c_Node.m_new.call(new c_Node,t_key,t_value,-1,t_parent);
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<47>";
	if((t_parent)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<48>";
		if(t_cmp>0){
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<49>";
			dbg_object(t_parent).m_right=t_node;
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<51>";
			dbg_object(t_parent).m_left=t_node;
		}
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<53>";
		this.p_InsertFixup(t_node);
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<55>";
		this.m_root=t_node;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<57>";
	pop_err();
	return true;
}
c_Map.prototype.p_Insert=function(t_key,t_value){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<146>";
	var t_=this.p_Set(t_key,t_value);
	pop_err();
	return t_;
}
function c_IntMap(){
	c_Map.call(this);
}
c_IntMap.prototype=extend_class(c_Map);
c_IntMap.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<534>";
	c_Map.m_new.call(this);
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<534>";
	pop_err();
	return this;
}
c_IntMap.prototype.p_Compare=function(t_lhs,t_rhs){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<537>";
	var t_=t_lhs-t_rhs;
	pop_err();
	return t_;
}
function c_Stack(){
	Object.call(this);
	this.m_data=[];
	this.m_length=0;
}
c_Stack.m_new=function(){
	push_err();
	pop_err();
	return this;
}
c_Stack.m_new2=function(t_data){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<13>";
	dbg_object(this).m_data=t_data.slice(0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<14>";
	dbg_object(this).m_length=t_data.length;
	pop_err();
	return this;
}
c_Stack.prototype.p_Push=function(t_value){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<71>";
	if(this.m_length==this.m_data.length){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<72>";
		this.m_data=resize_object_array(this.m_data,this.m_length*2+10);
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<74>";
	dbg_array(this.m_data,this.m_length)[dbg_index]=t_value;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<75>";
	this.m_length+=1;
	pop_err();
}
c_Stack.prototype.p_Push2=function(t_values,t_offset,t_count){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<83>";
	for(var t_i=0;t_i<t_count;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<84>";
		this.p_Push(dbg_array(t_values,t_offset+t_i)[dbg_index]);
	}
	pop_err();
}
c_Stack.prototype.p_Push3=function(t_values,t_offset){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<79>";
	this.p_Push2(t_values,t_offset,t_values.length-t_offset);
	pop_err();
}
c_Stack.prototype.p_ToArray=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<18>";
	var t_t=new_object_array(this.m_length);
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<19>";
	for(var t_i=0;t_i<this.m_length;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<20>";
		dbg_array(t_t,t_i)[dbg_index]=dbg_array(this.m_data,t_i)[dbg_index];
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/stack.monkey<22>";
	pop_err();
	return t_t;
}
function c_Node(){
	Object.call(this);
	this.m_key=0;
	this.m_right=null;
	this.m_left=null;
	this.m_value=null;
	this.m_color=0;
	this.m_parent=null;
}
c_Node.m_new=function(t_key,t_value,t_color,t_parent){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<364>";
	dbg_object(this).m_key=t_key;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<365>";
	dbg_object(this).m_value=t_value;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<366>";
	dbg_object(this).m_color=t_color;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<367>";
	dbg_object(this).m_parent=t_parent;
	pop_err();
	return this;
}
c_Node.m_new2=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/map.monkey<361>";
	pop_err();
	return this;
}
var bb_app__displayModes=[];
var bb_app__desktopMode=null;
function bb_app_DeviceWidth(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<263>";
	pop_err();
	return bb_app__devWidth;
}
function bb_app_DeviceHeight(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<267>";
	pop_err();
	return bb_app__devHeight;
}
function bb_app_EnumDisplayModes(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<33>";
	var t_modes=bb_app__game.GetDisplayModes();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<34>";
	var t_mmap=c_IntMap.m_new.call(new c_IntMap);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<35>";
	var t_mstack=c_Stack.m_new.call(new c_Stack);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<36>";
	for(var t_i=0;t_i<t_modes.length;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<37>";
		var t_w=dbg_object(dbg_array(t_modes,t_i)[dbg_index]).width;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<38>";
		var t_h=dbg_object(dbg_array(t_modes,t_i)[dbg_index]).height;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<39>";
		var t_size=t_w<<16|t_h;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<40>";
		if(t_mmap.p_Contains(t_size)){
		}else{
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<42>";
			var t_mode=c_DisplayMode.m_new.call(new c_DisplayMode,dbg_object(dbg_array(t_modes,t_i)[dbg_index]).width,dbg_object(dbg_array(t_modes,t_i)[dbg_index]).height);
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<43>";
			t_mmap.p_Insert(t_size,t_mode);
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<44>";
			t_mstack.p_Push(t_mode);
		}
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<47>";
	bb_app__displayModes=t_mstack.p_ToArray();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<48>";
	var t_mode2=bb_app__game.GetDesktopMode();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<49>";
	if((t_mode2)!=null){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<50>";
		bb_app__desktopMode=c_DisplayMode.m_new.call(new c_DisplayMode,dbg_object(t_mode2).width,dbg_object(t_mode2).height);
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<52>";
		bb_app__desktopMode=c_DisplayMode.m_new.call(new c_DisplayMode,bb_app_DeviceWidth(),bb_app_DeviceHeight());
	}
	pop_err();
}
var bb_graphics_renderDevice=null;
function bb_graphics_SetMatrix(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<312>";
	dbg_object(bb_graphics_context).m_ix=t_ix;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<313>";
	dbg_object(bb_graphics_context).m_iy=t_iy;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<314>";
	dbg_object(bb_graphics_context).m_jx=t_jx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<315>";
	dbg_object(bb_graphics_context).m_jy=t_jy;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<316>";
	dbg_object(bb_graphics_context).m_tx=t_tx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<317>";
	dbg_object(bb_graphics_context).m_ty=t_ty;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<318>";
	dbg_object(bb_graphics_context).m_tformed=((t_ix!=1.0 || t_iy!=0.0 || t_jx!=0.0 || t_jy!=1.0 || t_tx!=0.0 || t_ty!=0.0)?1:0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<319>";
	dbg_object(bb_graphics_context).m_matDirty=1;
	pop_err();
	return 0;
}
function bb_graphics_SetMatrix2(t_m){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<308>";
	bb_graphics_SetMatrix(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_SetColor(t_r,t_g,t_b){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<254>";
	dbg_object(bb_graphics_context).m_color_r=t_r;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<255>";
	dbg_object(bb_graphics_context).m_color_g=t_g;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<256>";
	dbg_object(bb_graphics_context).m_color_b=t_b;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<257>";
	bb_graphics_renderDevice.SetColor(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_graphics_SetAlpha(t_alpha){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<271>";
	dbg_object(bb_graphics_context).m_alpha=t_alpha;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<272>";
	bb_graphics_renderDevice.SetAlpha(t_alpha);
	pop_err();
	return 0;
}
function bb_graphics_SetBlend(t_blend){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<280>";
	dbg_object(bb_graphics_context).m_blend=t_blend;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<281>";
	bb_graphics_renderDevice.SetBlend(t_blend);
	pop_err();
	return 0;
}
function bb_graphics_SetScissor(t_x,t_y,t_width,t_height){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<289>";
	dbg_object(bb_graphics_context).m_scissor_x=t_x;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<290>";
	dbg_object(bb_graphics_context).m_scissor_y=t_y;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<291>";
	dbg_object(bb_graphics_context).m_scissor_width=t_width;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<292>";
	dbg_object(bb_graphics_context).m_scissor_height=t_height;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<293>";
	bb_graphics_renderDevice.SetScissor(((t_x)|0),((t_y)|0),((t_width)|0),((t_height)|0));
	pop_err();
	return 0;
}
function bb_graphics_BeginRender(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<225>";
	bb_graphics_renderDevice=bb_graphics_device;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<226>";
	dbg_object(bb_graphics_context).m_matrixSp=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<227>";
	bb_graphics_SetMatrix(1.0,0.0,0.0,1.0,0.0,0.0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<228>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<229>";
	bb_graphics_SetAlpha(1.0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<230>";
	bb_graphics_SetBlend(0);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<231>";
	bb_graphics_SetScissor(0.0,0.0,(bb_app_DeviceWidth()),(bb_app_DeviceHeight()));
	pop_err();
	return 0;
}
function bb_graphics_EndRender(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<235>";
	bb_graphics_renderDevice=null;
	pop_err();
	return 0;
}
function c_BBGameEvent(){
	Object.call(this);
}
function bb_app_EndApp(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<259>";
	error("");
	pop_err();
}
function c_RoomMap(){
	Object.call(this);
	this.m_MapWidth=6;
	this.m_MapHeight=6;
	this.m_Map=new_array_array(6);
	this.m_currentRoom=null;
	this.m_roomNum=15;
	this.m_MapPSize=32;
	this.m_MapXOffset=0;
}
c_RoomMap.m_new=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<229>";
	pop_err();
	return this;
}
c_RoomMap.prototype.p_Build=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<239>";
	var t_RoomCount=0;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<241>";
	for(var t_i=0;t_i<=this.m_MapWidth-1;t_i=t_i+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<242>";
		dbg_array(this.m_Map,t_i)[dbg_index]=new_object_array(this.m_MapHeight);
	}
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<246>";
	for(var t_i2=0;t_i2<=this.m_MapWidth-1;t_i2=t_i2+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<247>";
		for(var t_j=0;t_j<=this.m_MapHeight-1;t_j=t_j+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<248>";
			dbg_array(dbg_array(this.m_Map,t_i2)[dbg_index],t_j)[dbg_index]=c_Room.m_new.call(new c_Room,0,t_i2,t_j);
		}
	}
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<254>";
	var t_xr=((bb_random_Rnd()*(this.m_MapWidth))|0);
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<255>";
	var t_yr=((bb_random_Rnd()*(this.m_MapHeight))|0);
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<256>";
	dbg_array(dbg_array(this.m_Map,t_xr)[dbg_index],t_yr)[dbg_index].p_UpdateType(1);
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<257>";
	this.m_currentRoom=dbg_array(dbg_array(this.m_Map,t_xr)[dbg_index],t_yr)[dbg_index];
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<258>";
	t_RoomCount+=1;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<260>";
	if(t_xr>0 && t_xr<this.m_MapWidth-1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<261>";
		dbg_array(dbg_array(this.m_Map,t_xr+1)[dbg_index],t_yr)[dbg_index].p_UpdateNeighbours(1);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<262>";
		dbg_array(dbg_array(this.m_Map,t_xr-1)[dbg_index],t_yr)[dbg_index].p_UpdateNeighbours(1);
	}else{
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<263>";
		if(t_xr==0){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<264>";
			dbg_array(dbg_array(this.m_Map,t_xr+1)[dbg_index],t_yr)[dbg_index].p_UpdateNeighbours(1);
		}else{
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<265>";
			if(t_xr==this.m_MapWidth-1){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<266>";
				dbg_array(dbg_array(this.m_Map,t_xr-1)[dbg_index],t_yr)[dbg_index].p_UpdateNeighbours(1);
			}
		}
	}
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<269>";
	if(t_yr>0 && t_yr<this.m_MapHeight-1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<270>";
		dbg_array(dbg_array(this.m_Map,t_xr)[dbg_index],t_yr+1)[dbg_index].p_UpdateNeighbours(1);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<271>";
		dbg_array(dbg_array(this.m_Map,t_xr)[dbg_index],t_yr-1)[dbg_index].p_UpdateNeighbours(1);
	}else{
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<272>";
		if(t_yr==0){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<273>";
			dbg_array(dbg_array(this.m_Map,t_xr)[dbg_index],t_yr+1)[dbg_index].p_UpdateNeighbours(1);
		}else{
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<274>";
			if(t_yr==this.m_MapHeight-1){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<275>";
				dbg_array(dbg_array(this.m_Map,t_xr)[dbg_index],t_yr-1)[dbg_index].p_UpdateNeighbours(1);
			}
		}
	}
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<279>";
	while(t_RoomCount<this.m_roomNum){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<280>";
		var t_Valid=c_List.m_new.call(new c_List);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<282>";
		for(var t_i3=0;t_i3<=this.m_MapWidth-1;t_i3=t_i3+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<283>";
			for(var t_j2=0;t_j2<=this.m_MapHeight-1;t_j2=t_j2+1){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<284>";
				if(dbg_array(dbg_array(this.m_Map,t_i3)[dbg_index],t_j2)[dbg_index].p_GetNeighbours()==1 && dbg_array(dbg_array(this.m_Map,t_i3)[dbg_index],t_j2)[dbg_index].p_GetType()==0){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<285>";
					t_Valid.p_AddLast(dbg_array(dbg_array(this.m_Map,t_i3)[dbg_index],t_j2)[dbg_index]);
				}
			}
		}
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<291>";
		var t_validRoomArray=t_Valid.p_ToArray();
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<294>";
		var t_ir=((bb_random_Rnd()*(t_validRoomArray.length)-1.0)|0);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<295>";
		dbg_array(t_validRoomArray,t_ir)[dbg_index].p_UpdateType(2);
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<296>";
		t_RoomCount+=1;
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<299>";
		var t_x=dbg_array(t_validRoomArray,t_ir)[dbg_index].p_GetX();
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<300>";
		var t_y=dbg_array(t_validRoomArray,t_ir)[dbg_index].p_GetY();
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<303>";
		if(t_x>0 && t_x<this.m_MapWidth-1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<304>";
			dbg_array(dbg_array(this.m_Map,t_x+1)[dbg_index],t_y)[dbg_index].p_UpdateNeighbours(1);
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<305>";
			dbg_array(dbg_array(this.m_Map,t_x-1)[dbg_index],t_y)[dbg_index].p_UpdateNeighbours(1);
		}else{
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<306>";
			if(t_x==0){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<307>";
				dbg_array(dbg_array(this.m_Map,t_x+1)[dbg_index],t_y)[dbg_index].p_UpdateNeighbours(1);
			}else{
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<308>";
				if(t_x==this.m_MapWidth-1){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<309>";
					dbg_array(dbg_array(this.m_Map,t_x-1)[dbg_index],t_y)[dbg_index].p_UpdateNeighbours(1);
				}
			}
		}
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<312>";
		if(t_y>0 && t_y<this.m_MapHeight-1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<313>";
			dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y+1)[dbg_index].p_UpdateNeighbours(1);
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<314>";
			dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y-1)[dbg_index].p_UpdateNeighbours(1);
		}else{
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<315>";
			if(t_y==0){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<316>";
				dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y+1)[dbg_index].p_UpdateNeighbours(1);
			}else{
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<317>";
				if(t_y==this.m_MapHeight-1){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<318>";
					dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y-1)[dbg_index].p_UpdateNeighbours(1);
				}
			}
		}
	}
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<324>";
	for(var t_x2=0;t_x2<=this.m_MapWidth-1;t_x2=t_x2+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<325>";
		for(var t_y2=0;t_y2<=this.m_MapHeight-1;t_y2=t_y2+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<326>";
			if(dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2)[dbg_index].p_GetType()!=0){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<327>";
				if(t_y2>0){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<328>";
					if(dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2-1)[dbg_index].p_GetType()!=0 && t_y2>0){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<329>";
						dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2)[dbg_index].p_SetnDoor("1");
					}
				}
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<332>";
				if(t_y2<this.m_MapHeight-1){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<333>";
					if(dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2+1)[dbg_index].p_GetType()!=0 && t_y2<this.m_MapHeight-1){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<334>";
						dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2)[dbg_index].p_SetsDoor("1");
					}
				}
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<337>";
				if(t_x2>0){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<338>";
					if(dbg_array(dbg_array(this.m_Map,t_x2-1)[dbg_index],t_y2)[dbg_index].p_GetType()!=0 && t_x2>0){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<339>";
						dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2)[dbg_index].p_SetwDoor("1");
					}
				}
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<342>";
				if(t_x2<this.m_MapWidth-1){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<343>";
					if(dbg_array(dbg_array(this.m_Map,t_x2+1)[dbg_index],t_y2)[dbg_index].p_GetType()!=0 && t_x2<this.m_MapWidth-1){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<344>";
						dbg_array(dbg_array(this.m_Map,t_x2)[dbg_index],t_y2)[dbg_index].p_SeteDoor("1");
					}
				}
			}
		}
	}
	pop_err();
	return 0;
}
c_RoomMap.prototype.p_Reset=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<444>";
	for(var t_x=0;t_x<=this.m_MapWidth-1;t_x=t_x+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<445>";
		for(var t_y=0;t_y<=this.m_MapHeight-1;t_y=t_y+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<446>";
			dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y)[dbg_index].p_Reset();
		}
	}
	pop_err();
	return 0;
}
c_RoomMap.prototype.p_Draw=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<354>";
	this.m_currentRoom.p_DrawWalls();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<355>";
	this.m_currentRoom.p_DrawFloor();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<357>";
	bb_graphics_Cls(0.0,0.0,0.0);
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<360>";
	for(var t_x=0;t_x<=this.m_MapWidth-1;t_x=t_x+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<361>";
		for(var t_y=0;t_y<=this.m_MapHeight-1;t_y=t_y+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<362>";
			var t_4=dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y)[dbg_index].p_GetType();
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<363>";
			if(t_4==0){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<364>";
				bb_graphics_SetColor(0.0,0.0,0.0);
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<365>";
				bb_graphics_DrawRect((t_x*this.m_MapPSize+this.m_MapXOffset),(t_y*this.m_MapPSize),(this.m_MapPSize),(this.m_MapPSize));
			}else{
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<366>";
				if(t_4==1){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<367>";
					bb_graphics_SetColor(255.0,255.0,255.0);
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<368>";
					bb_graphics_DrawRect((t_x*this.m_MapPSize+this.m_MapXOffset),(t_y*this.m_MapPSize),(this.m_MapPSize),(this.m_MapPSize));
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<369>";
					var t_5=dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y)[dbg_index].p_GetDoors();
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<370>";
					if(t_5=="1000"){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<371>";
						bb_LayeredLevel_DrawRoom("StartRoom/N.png",t_x,t_y,this.m_MapPSize);
					}else{
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<372>";
						if(t_5=="0100"){
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<373>";
							bb_LayeredLevel_DrawRoom("StartRoom/E.png",t_x,t_y,this.m_MapPSize);
						}else{
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<374>";
							if(t_5=="0010"){
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<375>";
								bb_LayeredLevel_DrawRoom("StartRoom/S.png",t_x,t_y,this.m_MapPSize);
							}else{
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<376>";
								if(t_5=="0001"){
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<377>";
									bb_LayeredLevel_DrawRoom("StartRoom/W.png",t_x,t_y,this.m_MapPSize);
								}else{
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<378>";
									if(t_5=="1100"){
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<379>";
										bb_LayeredLevel_DrawRoom("StartRoom/NE.png",t_x,t_y,this.m_MapPSize);
									}else{
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<380>";
										if(t_5=="1010"){
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<381>";
											bb_LayeredLevel_DrawRoom("StartRoom/NS.png",t_x,t_y,this.m_MapPSize);
										}else{
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<382>";
											if(t_5=="1001"){
												err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<383>";
												bb_LayeredLevel_DrawRoom("StartRoom/NW.png",t_x,t_y,this.m_MapPSize);
											}else{
												err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<384>";
												if(t_5=="1110"){
													err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<385>";
													bb_LayeredLevel_DrawRoom("StartRoom/NES.png",t_x,t_y,this.m_MapPSize);
												}else{
													err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<386>";
													if(t_5=="1101"){
														err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<387>";
														bb_LayeredLevel_DrawRoom("StartRoom/NEW.png",t_x,t_y,this.m_MapPSize);
													}else{
														err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<388>";
														if(t_5=="0110"){
															err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<389>";
															bb_LayeredLevel_DrawRoom("StartRoom/ES.png",t_x,t_y,this.m_MapPSize);
														}else{
															err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<390>";
															if(t_5=="0011"){
																err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<391>";
																bb_LayeredLevel_DrawRoom("StartRoom/SW.png",t_x,t_y,this.m_MapPSize);
															}else{
																err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<392>";
																if(t_5=="0111"){
																	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<393>";
																	bb_LayeredLevel_DrawRoom("StartRoom/ESW.png",t_x,t_y,this.m_MapPSize);
																}else{
																	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<394>";
																	if(t_5=="1011"){
																		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<395>";
																		bb_LayeredLevel_DrawRoom("StartRoom/NSW.png",t_x,t_y,this.m_MapPSize);
																	}else{
																		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<396>";
																		if(t_5=="0101"){
																			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<397>";
																			bb_LayeredLevel_DrawRoom("StartRoom/EW.png",t_x,t_y,this.m_MapPSize);
																		}else{
																			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<398>";
																			if(t_5=="1111"){
																				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<399>";
																				bb_LayeredLevel_DrawRoom("StartRoom/NESW.png",t_x,t_y,this.m_MapPSize);
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
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<401>";
					if(t_4==2){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<402>";
						bb_graphics_SetColor(255.0,255.0,255.0);
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<403>";
						bb_graphics_DrawRect((t_x*this.m_MapPSize+this.m_MapXOffset),(t_y*this.m_MapPSize),(this.m_MapPSize),(this.m_MapPSize));
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<404>";
						var t_6=dbg_array(dbg_array(this.m_Map,t_x)[dbg_index],t_y)[dbg_index].p_GetDoors();
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<405>";
						if(t_6=="1000"){
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<406>";
							bb_LayeredLevel_DrawRoom("Room/N.png",t_x,t_y,this.m_MapPSize);
						}else{
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<407>";
							if(t_6=="0100"){
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<408>";
								bb_LayeredLevel_DrawRoom("Room/E.png",t_x,t_y,this.m_MapPSize);
							}else{
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<409>";
								if(t_6=="0010"){
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<410>";
									bb_LayeredLevel_DrawRoom("Room/S.png",t_x,t_y,this.m_MapPSize);
								}else{
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<411>";
									if(t_6=="0001"){
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<412>";
										bb_LayeredLevel_DrawRoom("Room/W.png",t_x,t_y,this.m_MapPSize);
									}else{
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<413>";
										if(t_6=="1100"){
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<414>";
											bb_LayeredLevel_DrawRoom("Room/NE.png",t_x,t_y,this.m_MapPSize);
										}else{
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<415>";
											if(t_6=="1010"){
												err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<416>";
												bb_LayeredLevel_DrawRoom("Room/NS.png",t_x,t_y,this.m_MapPSize);
											}else{
												err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<417>";
												if(t_6=="1001"){
													err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<418>";
													bb_LayeredLevel_DrawRoom("Room/NW.png",t_x,t_y,this.m_MapPSize);
												}else{
													err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<419>";
													if(t_6=="1110"){
														err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<420>";
														bb_LayeredLevel_DrawRoom("Room/NES.png",t_x,t_y,this.m_MapPSize);
													}else{
														err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<421>";
														if(t_6=="1101"){
															err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<422>";
															bb_LayeredLevel_DrawRoom("Room/NEW.png",t_x,t_y,this.m_MapPSize);
														}else{
															err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<423>";
															if(t_6=="0110"){
																err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<424>";
																bb_LayeredLevel_DrawRoom("Room/ES.png",t_x,t_y,this.m_MapPSize);
															}else{
																err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<425>";
																if(t_6=="0011"){
																	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<426>";
																	bb_LayeredLevel_DrawRoom("Room/SW.png",t_x,t_y,this.m_MapPSize);
																}else{
																	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<427>";
																	if(t_6=="0111"){
																		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<428>";
																		bb_LayeredLevel_DrawRoom("Room/ESW.png",t_x,t_y,this.m_MapPSize);
																	}else{
																		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<429>";
																		if(t_6=="1011"){
																			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<430>";
																			bb_LayeredLevel_DrawRoom("Room/NSW.png",t_x,t_y,this.m_MapPSize);
																		}else{
																			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<431>";
																			if(t_6=="0101"){
																				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<432>";
																				bb_LayeredLevel_DrawRoom("Room/EW.png",t_x,t_y,this.m_MapPSize);
																			}else{
																				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<433>";
																				if(t_6=="1111"){
																					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<434>";
																					bb_LayeredLevel_DrawRoom("Room/NESW.png",t_x,t_y,this.m_MapPSize);
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
	pop_err();
	return 0;
}
function bb_input_KeyHit(t_key){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/input.monkey<44>";
	var t_=bb_input_device.p_KeyHit(t_key);
	pop_err();
	return t_;
}
function bb_app_Millisecs(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/app.monkey<233>";
	var t_=bb_app__game.Millisecs();
	pop_err();
	return t_;
}
var bb_random_Seed=0;
function c_Room(){
	Object.call(this);
	this.m_x=0;
	this.m_y=0;
	this.m_Type=0;
	this.m_WallLayout=[];
	this.m_FloorLayout=[];
	this.m_CollisionArray=[];
	this.m_Neighbours=0;
	this.m_nDoor="0";
	this.m_sDoor="0";
	this.m_wDoor="0";
	this.m_eDoor="0";
	this.m_RoomSize32=15;
	this.m_RoomTiles=bb_graphics_LoadImage("ProtoTileSet.png",1,c_Image.m_DefaultFlags);
	this.m_DoorArray=[];
}
c_Room.m_new=function(t_Type,t_X,t_Y){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<71>";
	dbg_object(this).m_x=t_X;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<72>";
	dbg_object(this).m_y=t_Y;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<73>";
	dbg_object(this).m_Type=t_Type;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<88>";
	this.m_WallLayout=[[0,1,1,1,1,1,1,1,1,1,1,1,1,1,2],[3,11,11,11,11,11,11,11,11,11,11,11,11,11,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,7,7,7,7,7,7,7,7,7,7,7,7,7,4],[5,10,10,10,10,10,10,10,10,10,10,10,10,10,6]];
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<103>";
	this.m_FloorLayout=[[0,1,1,1,1,1,1,1,1,1,1,1,1,1,2],[3,9,9,9,9,9,9,9,9,9,9,9,9,9,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[3,8,8,8,8,8,8,8,8,8,8,8,8,8,4],[5,10,10,10,10,10,10,10,10,10,10,10,10,10,6]];
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<118>";
	this.m_CollisionArray=[[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,0,0,0,0,0,0,0,0,0,0,0,0,0,1],[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]];
	pop_err();
	return this;
}
c_Room.m_new2=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<54>";
	pop_err();
	return this;
}
c_Room.prototype.p_UpdateType=function(t_Type){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<199>";
	dbg_object(this).m_Type=t_Type;
	pop_err();
	return 0;
}
c_Room.prototype.p_UpdateNeighbours=function(t_Amount){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<195>";
	dbg_object(this).m_Neighbours+=t_Amount;
	pop_err();
	return 0;
}
c_Room.prototype.p_GetNeighbours=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<203>";
	pop_err();
	return this.m_Neighbours;
}
c_Room.prototype.p_GetType=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<207>";
	pop_err();
	return this.m_Type;
}
c_Room.prototype.p_GetX=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<211>";
	pop_err();
	return dbg_object(this).m_x;
}
c_Room.prototype.p_GetY=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<215>";
	pop_err();
	return dbg_object(this).m_y;
}
c_Room.prototype.p_SetnDoor=function(t__nDoor){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<178>";
	dbg_object(this).m_nDoor=t__nDoor;
	pop_err();
	return 0;
}
c_Room.prototype.p_SetsDoor=function(t__sDoor){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<182>";
	dbg_object(this).m_sDoor=t__sDoor;
	pop_err();
	return 0;
}
c_Room.prototype.p_SetwDoor=function(t__wDoor){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<190>";
	dbg_object(this).m_wDoor=t__wDoor;
	pop_err();
	return 0;
}
c_Room.prototype.p_SeteDoor=function(t__eDoor){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<186>";
	dbg_object(this).m_eDoor=t__eDoor;
	pop_err();
	return 0;
}
c_Room.prototype.p_Reset=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<218>";
	dbg_object(this).m_Type=0;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<219>";
	dbg_object(this).m_Neighbours=0;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<220>";
	dbg_object(this).m_nDoor="0";
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<221>";
	dbg_object(this).m_eDoor="0";
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<222>";
	dbg_object(this).m_sDoor="0";
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<223>";
	dbg_object(this).m_wDoor="0";
	pop_err();
	return 0;
}
c_Room.prototype.p_DrawWalls=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<137>";
	for(var t_x=0;t_x<=this.m_RoomSize32-1;t_x=t_x+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<138>";
		for(var t_y=0;t_y<=this.m_RoomSize32-1;t_y=t_y+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<139>";
			var t_3=dbg_array(dbg_array(this.m_WallLayout,t_y)[dbg_index],t_x)[dbg_index];
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<140>";
			if(t_3==0){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<141>";
				var t_topLeftWall=this.m_RoomTiles.p_GrabImage(0,320,32,32,1,c_Image.m_DefaultFlags);
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<142>";
				bb_graphics_DrawImage(t_topLeftWall,(t_x*32),(t_y*32),0);
			}else{
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<143>";
				if(t_3==1){
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<144>";
					var t_wall=this.m_RoomTiles.p_GrabImage(32,320,32,32,1,c_Image.m_DefaultFlags);
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<145>";
					bb_graphics_DrawImage(t_wall,(t_x*32),(t_y*32),0);
				}else{
					err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<146>";
					if(t_3==2){
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<147>";
						var t_topRightWall=this.m_RoomTiles.p_GrabImage(64,320,32,32,1,c_Image.m_DefaultFlags);
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<148>";
						bb_graphics_DrawImage(t_topRightWall,(t_x*32),(t_y*32),0);
					}else{
						err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<149>";
						if(t_3==3){
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<150>";
							var t_leftWall=this.m_RoomTiles.p_GrabImage(0,352,32,32,1,c_Image.m_DefaultFlags);
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<151>";
							bb_graphics_DrawImage(t_leftWall,(t_x*32),(t_y*32),0);
						}else{
							err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<152>";
							if(t_3==4){
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<153>";
								var t_rightWall=this.m_RoomTiles.p_GrabImage(64,352,32,32,1,c_Image.m_DefaultFlags);
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<154>";
								bb_graphics_DrawImage(t_rightWall,(t_x*32),(t_y*32),0);
							}else{
								err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<155>";
								if(t_3==5){
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<156>";
									var t_botLeftWall=this.m_RoomTiles.p_GrabImage(0,416,32,32,1,c_Image.m_DefaultFlags);
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<157>";
									bb_graphics_DrawImage(t_botLeftWall,(t_x*32),(t_y*32),0);
								}else{
									err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<158>";
									if(t_3==6){
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<159>";
										var t_botRightWall=this.m_RoomTiles.p_GrabImage(64,416,32,32,1,c_Image.m_DefaultFlags);
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<160>";
										bb_graphics_DrawImage(t_botRightWall,(t_x*32),(t_y*32),0);
									}else{
										err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<161>";
										if(t_3==7){
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<162>";
											var t_botWall=this.m_RoomTiles.p_GrabImage(32,384,32,32,1,c_Image.m_DefaultFlags);
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<163>";
											bb_graphics_DrawImage(t_botWall,(t_x*32),(t_y*32),0);
										}else{
											err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<164>";
											if(t_3==10){
												err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<165>";
												var t_botBrick=this.m_RoomTiles.p_GrabImage(32,416,32,32,1,c_Image.m_DefaultFlags);
												err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<166>";
												bb_graphics_DrawImage(t_botBrick,(t_x*32),(t_y*32),0);
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
	pop_err();
	return 0;
}
c_Room.prototype.p_DrawFloor=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<122>";
	for(var t_x=0;t_x<=this.m_RoomSize32-1;t_x=t_x+1){
		err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<123>";
		for(var t_y=0;t_y<=this.m_RoomSize32-1;t_y=t_y+1){
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<124>";
			if(dbg_array(dbg_array(this.m_FloorLayout,t_y)[dbg_index],t_x)[dbg_index]==8){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<125>";
				var t_floor=this.m_RoomTiles.p_GrabImage(0,288,32,32,1,c_Image.m_DefaultFlags);
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<126>";
				bb_graphics_DrawImage(t_floor,(t_x*32),(t_y*32),0);
			}
			err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<128>";
			if(dbg_array(dbg_array(this.m_FloorLayout,t_y)[dbg_index],t_x)[dbg_index]==9){
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<129>";
				var t_topBrick=this.m_RoomTiles.p_GrabImage(32,352,32,32,1,c_Image.m_DefaultFlags);
				err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<130>";
				bb_graphics_DrawImage(t_topBrick,(t_x*32),(t_y*32),0);
			}
		}
	}
	pop_err();
	return 0;
}
c_Room.prototype.p_GetDoors=function(){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<173>";
	this.m_DoorArray=[this.m_nDoor,this.m_eDoor,this.m_sDoor,this.m_wDoor];
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<174>";
	var t_=this.m_DoorArray.join("");
	pop_err();
	return t_;
}
function bb_random_Rnd(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/random.monkey<21>";
	bb_random_Seed=bb_random_Seed*1664525+1013904223|0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/random.monkey<22>";
	var t_=(bb_random_Seed>>8&16777215)/16777216.0;
	pop_err();
	return t_;
}
function bb_random_Rnd2(t_low,t_high){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/random.monkey<30>";
	var t_=bb_random_Rnd3(t_high-t_low)+t_low;
	pop_err();
	return t_;
}
function bb_random_Rnd3(t_range){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/random.monkey<26>";
	var t_=bb_random_Rnd()*t_range;
	pop_err();
	return t_;
}
function c_List(){
	Object.call(this);
	this.m__head=(c_HeadNode.m_new.call(new c_HeadNode));
}
c_List.m_new=function(){
	push_err();
	pop_err();
	return this;
}
c_List.prototype.p_AddLast=function(t_data){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<108>";
	var t_=c_Node2.m_new.call(new c_Node2,this.m__head,dbg_object(this.m__head).m__pred,t_data);
	pop_err();
	return t_;
}
c_List.m_new2=function(t_data){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<13>";
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<13>";
	var t_=t_data;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<13>";
	var t_2=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<13>";
	while(t_2<t_.length){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<13>";
		var t_t=dbg_array(t_,t_2)[dbg_index];
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<13>";
		t_2=t_2+1;
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<14>";
		this.p_AddLast(t_t);
	}
	pop_err();
	return this;
}
c_List.prototype.p_Count=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<41>";
	var t_n=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<41>";
	var t_node=dbg_object(this.m__head).m__succ;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<42>";
	while(t_node!=this.m__head){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<43>";
		t_node=dbg_object(t_node).m__succ;
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<44>";
		t_n+=1;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<46>";
	pop_err();
	return t_n;
}
c_List.prototype.p_ObjectEnumerator=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<186>";
	var t_=c_Enumerator.m_new.call(new c_Enumerator,this);
	pop_err();
	return t_;
}
c_List.prototype.p_ToArray=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<19>";
	var t_arr=new_object_array(this.p_Count());
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<19>";
	var t_i=0;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<20>";
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<20>";
	var t_=this.p_ObjectEnumerator();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<20>";
	while(t_.p_HasNext()){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<20>";
		var t_t=t_.p_NextObject();
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<21>";
		dbg_array(t_arr,t_i)[dbg_index]=t_t;
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<22>";
		t_i+=1;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<24>";
	pop_err();
	return t_arr;
}
function c_Node2(){
	Object.call(this);
	this.m__succ=null;
	this.m__pred=null;
	this.m__data=null;
}
c_Node2.m_new=function(t_succ,t_pred,t_data){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<261>";
	this.m__succ=t_succ;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<262>";
	this.m__pred=t_pred;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<263>";
	dbg_object(this.m__succ).m__pred=this;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<264>";
	dbg_object(this.m__pred).m__succ=this;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<265>";
	this.m__data=t_data;
	pop_err();
	return this;
}
c_Node2.m_new2=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<258>";
	pop_err();
	return this;
}
function c_HeadNode(){
	c_Node2.call(this);
}
c_HeadNode.prototype=extend_class(c_Node2);
c_HeadNode.m_new=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<310>";
	c_Node2.m_new2.call(this);
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<311>";
	this.m__succ=(this);
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<312>";
	this.m__pred=(this);
	pop_err();
	return this;
}
function c_Enumerator(){
	Object.call(this);
	this.m__list=null;
	this.m__curr=null;
}
c_Enumerator.m_new=function(t_list){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<326>";
	this.m__list=t_list;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<327>";
	this.m__curr=dbg_object(dbg_object(t_list).m__head).m__succ;
	pop_err();
	return this;
}
c_Enumerator.m_new2=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<323>";
	pop_err();
	return this;
}
c_Enumerator.prototype.p_HasNext=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<331>";
	while(dbg_object(dbg_object(this.m__curr).m__succ).m__pred!=this.m__curr){
		err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<332>";
		this.m__curr=dbg_object(this.m__curr).m__succ;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<334>";
	var t_=this.m__curr!=dbg_object(this.m__list).m__head;
	pop_err();
	return t_;
}
c_Enumerator.prototype.p_NextObject=function(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<338>";
	var t_data=dbg_object(this.m__curr).m__data;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<339>";
	this.m__curr=dbg_object(this.m__curr).m__succ;
	err_info="C:/Program Files (x86)/MonkeyX/modules/monkey/list.monkey<340>";
	pop_err();
	return t_data;
}
function bb_graphics_DebugRenderDevice(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<53>";
	if(!((bb_graphics_renderDevice)!=null)){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<53>";
		error("Rendering operations can only be performed inside OnRender");
	}
	pop_err();
	return 0;
}
function bb_graphics_Cls(t_r,t_g,t_b){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<378>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<380>";
	bb_graphics_renderDevice.Cls(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_graphics_DrawImage(t_image,t_x,t_y,t_frame){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<452>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<453>";
	if(t_frame<0 || t_frame>=dbg_object(t_image).m_frames.length){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<453>";
		error("Invalid image frame");
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<456>";
	var t_f=dbg_array(dbg_object(t_image).m_frames,t_frame)[dbg_index];
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<458>";
	bb_graphics_context.p_Validate();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<460>";
	if((dbg_object(t_image).m_flags&65536)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<461>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).m_surface,t_x-dbg_object(t_image).m_tx,t_y-dbg_object(t_image).m_ty);
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<463>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).m_surface,t_x-dbg_object(t_image).m_tx,t_y-dbg_object(t_image).m_ty,dbg_object(t_f).m_x,dbg_object(t_f).m_y,dbg_object(t_image).m_width,dbg_object(t_image).m_height);
	}
	pop_err();
	return 0;
}
function bb_graphics_PushMatrix(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<333>";
	var t_sp=dbg_object(bb_graphics_context).m_matrixSp;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<334>";
	if(t_sp==dbg_object(bb_graphics_context).m_matrixStack.length){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<334>";
		dbg_object(bb_graphics_context).m_matrixStack=resize_number_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp*2);
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<335>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+0)[dbg_index]=dbg_object(bb_graphics_context).m_ix;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<336>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+1)[dbg_index]=dbg_object(bb_graphics_context).m_iy;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<337>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+2)[dbg_index]=dbg_object(bb_graphics_context).m_jx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<338>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+3)[dbg_index]=dbg_object(bb_graphics_context).m_jy;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<339>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+4)[dbg_index]=dbg_object(bb_graphics_context).m_tx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<340>";
	dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+5)[dbg_index]=dbg_object(bb_graphics_context).m_ty;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<341>";
	dbg_object(bb_graphics_context).m_matrixSp=t_sp+6;
	pop_err();
	return 0;
}
function bb_graphics_Transform(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<355>";
	var t_ix2=t_ix*dbg_object(bb_graphics_context).m_ix+t_iy*dbg_object(bb_graphics_context).m_jx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<356>";
	var t_iy2=t_ix*dbg_object(bb_graphics_context).m_iy+t_iy*dbg_object(bb_graphics_context).m_jy;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<357>";
	var t_jx2=t_jx*dbg_object(bb_graphics_context).m_ix+t_jy*dbg_object(bb_graphics_context).m_jx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<358>";
	var t_jy2=t_jx*dbg_object(bb_graphics_context).m_iy+t_jy*dbg_object(bb_graphics_context).m_jy;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<359>";
	var t_tx2=t_tx*dbg_object(bb_graphics_context).m_ix+t_ty*dbg_object(bb_graphics_context).m_jx+dbg_object(bb_graphics_context).m_tx;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<360>";
	var t_ty2=t_tx*dbg_object(bb_graphics_context).m_iy+t_ty*dbg_object(bb_graphics_context).m_jy+dbg_object(bb_graphics_context).m_ty;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<361>";
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	pop_err();
	return 0;
}
function bb_graphics_Transform2(t_m){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<351>";
	bb_graphics_Transform(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_Translate(t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<365>";
	bb_graphics_Transform(1.0,0.0,0.0,1.0,t_x,t_y);
	pop_err();
	return 0;
}
function bb_graphics_Rotate(t_angle){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<373>";
	bb_graphics_Transform(Math.cos((t_angle)*D2R),-Math.sin((t_angle)*D2R),Math.sin((t_angle)*D2R),Math.cos((t_angle)*D2R),0.0,0.0);
	pop_err();
	return 0;
}
function bb_graphics_Scale(t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<369>";
	bb_graphics_Transform(t_x,0.0,0.0,t_y,0.0,0.0);
	pop_err();
	return 0;
}
function bb_graphics_PopMatrix(){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<345>";
	var t_sp=dbg_object(bb_graphics_context).m_matrixSp-6;
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<346>";
	bb_graphics_SetMatrix(dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+0)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+1)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+2)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+3)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+4)[dbg_index],dbg_array(dbg_object(bb_graphics_context).m_matrixStack,t_sp+5)[dbg_index]);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<347>";
	dbg_object(bb_graphics_context).m_matrixSp=t_sp;
	pop_err();
	return 0;
}
function bb_graphics_DrawImage2(t_image,t_x,t_y,t_rotation,t_scaleX,t_scaleY,t_frame){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<470>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<471>";
	if(t_frame<0 || t_frame>=dbg_object(t_image).m_frames.length){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<471>";
		error("Invalid image frame");
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<474>";
	var t_f=dbg_array(dbg_object(t_image).m_frames,t_frame)[dbg_index];
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<476>";
	bb_graphics_PushMatrix();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<478>";
	bb_graphics_Translate(t_x,t_y);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<479>";
	bb_graphics_Rotate(t_rotation);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<480>";
	bb_graphics_Scale(t_scaleX,t_scaleY);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<482>";
	bb_graphics_Translate(-dbg_object(t_image).m_tx,-dbg_object(t_image).m_ty);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<484>";
	bb_graphics_context.p_Validate();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<486>";
	if((dbg_object(t_image).m_flags&65536)!=0){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<487>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).m_surface,0.0,0.0);
	}else{
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<489>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).m_surface,0.0,0.0,dbg_object(t_f).m_x,dbg_object(t_f).m_y,dbg_object(t_image).m_width,dbg_object(t_image).m_height);
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<492>";
	bb_graphics_PopMatrix();
	pop_err();
	return 0;
}
function bb_graphics_DrawText(t_text,t_x,t_y,t_xalign,t_yalign){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<577>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<579>";
	if(!((dbg_object(bb_graphics_context).m_font)!=null)){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<579>";
		pop_err();
		return 0;
	}
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<581>";
	var t_w=dbg_object(bb_graphics_context).m_font.p_Width();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<582>";
	var t_h=dbg_object(bb_graphics_context).m_font.p_Height();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<584>";
	t_x-=Math.floor((t_w*t_text.length)*t_xalign);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<585>";
	t_y-=Math.floor((t_h)*t_yalign);
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<587>";
	for(var t_i=0;t_i<t_text.length;t_i=t_i+1){
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<588>";
		var t_ch=dbg_charCodeAt(t_text,t_i)-dbg_object(bb_graphics_context).m_firstChar;
		err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<589>";
		if(t_ch>=0 && t_ch<dbg_object(bb_graphics_context).m_font.p_Frames()){
			err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<590>";
			bb_graphics_DrawImage(dbg_object(bb_graphics_context).m_font,t_x+(t_i*t_w),t_y,t_ch);
		}
	}
	pop_err();
	return 0;
}
function bb_graphics_DrawRect(t_x,t_y,t_w,t_h){
	push_err();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<393>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<395>";
	bb_graphics_context.p_Validate();
	err_info="C:/Program Files (x86)/MonkeyX/modules/mojo/graphics.monkey<396>";
	bb_graphics_renderDevice.DrawRect(t_x,t_y,t_w,t_h);
	pop_err();
	return 0;
}
function bb_LayeredLevel_DrawRoom(t_path,t_x,t_y,t_mapPSize){
	push_err();
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<453>";
	var t_Room=null;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<454>";
	var t_ImageSize=32;
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<455>";
	t_Room=bb_graphics_LoadImage2(t_path,t_ImageSize,t_ImageSize,1,c_Image.m_DefaultFlags);
	err_info="X:/Game Prototypes/Level Drawing v2/LayeredLevel.monkey<456>";
	bb_graphics_DrawImage(t_Room,(t_x*t_mapPSize+0),(t_y*t_mapPSize),0);
	pop_err();
	return 0;
}
function bbInit(){
	bb_app__app=null;
	bb_app__delegate=null;
	bb_app__game=BBGame.Game();
	bb_LayeredLevel_Prototype=null;
	bb_graphics_device=null;
	bb_graphics_context=c_GraphicsContext.m_new.call(new c_GraphicsContext);
	c_Image.m_DefaultFlags=0;
	bb_audio_device=null;
	bb_input_device=null;
	bb_app__devWidth=0;
	bb_app__devHeight=0;
	bb_app__displayModes=[];
	bb_app__desktopMode=null;
	bb_graphics_renderDevice=null;
	bb_random_Seed=1234;
}
//${TRANSCODE_END}
