/*
 * Copyright (C) 2009 jeyries@yahoo.fr
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.jeyries.quake2;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnDismissListener;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.EGLConfigChooser;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock; 
import android.os.Vibrator;
import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.inputmethod.InputMethodManager;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Toast;

public class Quake2 extends Activity 
	implements SensorEventListener, Handler.Callback
{
	private QuakeView mGLSurfaceView = null;
	private QuakeRenderer mRenderer = new QuakeRenderer();
	
	private DownloadTask mDownloadTask = null;
    
    private ProgressDialog pd_loading = null;
    
	private Vibrator vibrator;
	
	private boolean please_exit = false;
	
	// android settings - saved as preferences
	private boolean debug = false,	
					invert_roll = false,
					enable_audio = true,
					enable_sensor = false,
					enable_vibrator = false,
					enable_ecomode = false;
	
    private long tstart;
    private int timelimit = 0; //4*60000;
    
    private String error_message;
    private int overlay = 0;
    
    public static final String version = "1.9" ;
    
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);           

        Log.i( "Quake2.java", "onCreate " + version);
        
        handlerUI  = new Handler(this);
        
        load_preferences();              
            
        //Log.i( "Quake2", "version : " + getVersion());
        
        // fullscreen
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
       
        // keep screen on 
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        
        show_main();
        
        // check for updates
        new Thread( new Runnable(){
			public void run() {
				check_updates();	
			}}).start();
    }
    
    
    /// Handler for asynchronous message
    /// => showDialog
    
    private Handler handlerUI ;
    
    public static final int MSG_SHOW_DIALOG = 1;
	
    
    // implements Handler.Callback
    //@Override
    public boolean handleMessage(Message msg) {

    	Log.i( "Quake2", String.format("handleMessage %d %d", msg.what, msg.arg1));
    	
    	switch( msg.what ){
    	
    	case MSG_SHOW_DIALOG:
    		showDialog(msg.arg1);
    		break;
    		
    	}
    	
    	return true;
    	
    }

    
    /////////////////////////////
    
    public void show_main() {
           
    	setContentView(R.layout.main);
        
    	TextView tv = (TextView)this.findViewById(R.id.textview_title);
    	tv.setText("Quake2 for Android v" + version);
    	
		Button button_start = (Button)this.findViewById(R.id.button_start);
        button_start.setOnClickListener( new OnClickListener() {  
        	             //@Override  
        	             public void onClick(View v) {  
        	            	 start_quake2();     	            	 
        	            	 
        	             }
					
        });  
        
    }
    
    
    public void show_settings() {

    	setContentView(R.layout.settings);

    	OnClickListener listener = new OnClickListener(){

    		public void onClick(View v) {

    			debug = ((CheckBox)findViewById(R.id.checkbox_debug))
    			.isChecked();

    			enable_audio = ((CheckBox)findViewById(R.id.checkbox_sound))
    			.isChecked();

    			enable_sensor = ((CheckBox)findViewById(R.id.checkbox_sensor))
    			.isChecked();

    			enable_vibrator = ((CheckBox)findViewById(R.id.checkbox_vibrator))
    			.isChecked();

    			enable_ecomode = ((CheckBox)findViewById(R.id.checkbox_ecomode))
    			.isChecked();


    			save_preferences(); 

    		}

    	};

    	CheckBox checkbox_debug = ((CheckBox)findViewById(R.id.checkbox_debug));
    	checkbox_debug.setChecked(debug);
    	checkbox_debug.setOnClickListener(listener);

    	CheckBox checkbox_sound = ((CheckBox)findViewById(R.id.checkbox_sound));
    	checkbox_sound.setChecked(enable_audio);
    	checkbox_sound.setOnClickListener(listener);

    	CheckBox checkbox_sensor = ((CheckBox)findViewById(R.id.checkbox_sensor));
    	checkbox_sensor.setChecked(enable_sensor);
    	checkbox_sensor.setOnClickListener(listener);

    	CheckBox checkbox_vibrator = ((CheckBox)findViewById(R.id.checkbox_vibrator));
    	checkbox_vibrator.setChecked(enable_vibrator);
    	checkbox_vibrator.setOnClickListener(listener);

    	CheckBox checkbox_ecomode = ((CheckBox)findViewById(R.id.checkbox_ecomode));
    	checkbox_ecomode.setChecked(enable_ecomode);
    	checkbox_ecomode.setOnClickListener(listener);



    }

    

    
    public void load_preferences(){

    	// Restore preferences
    	SharedPreferences settings = getPreferences( MODE_PRIVATE );
    	debug = settings.getBoolean("debug", debug);
    	invert_roll = settings.getBoolean("invert_roll", invert_roll);
    	enable_audio = settings.getBoolean("enable_audio", enable_audio );
    	enable_sensor = settings.getBoolean("enable_sensor", enable_sensor);
    	enable_vibrator = settings.getBoolean("enable_vibrator", enable_vibrator);
    	enable_ecomode = settings.getBoolean("enable_ecomode", enable_ecomode);   

    }

    public void save_preferences(){

    	// Save user preferences. We need an Editor object to
    	// make changes. All objects are from android.context.Context
    	SharedPreferences settings = getPreferences( MODE_PRIVATE );
    	SharedPreferences.Editor editor = settings.edit();

    	editor.putBoolean("debug", debug);
    	editor.putBoolean("invert_roll", invert_roll);
    	editor.putBoolean("enable_audio", enable_audio);
    	editor.putBoolean("enable_sensor", enable_sensor);
    	editor.putBoolean("enable_vibrator", enable_vibrator);
    	editor.putBoolean("enable_ecomode", enable_ecomode); 

    	// Don't forget to commit your edits!!!
    	editor.commit();



    }
    

   

    
    public void check_updates(){
 
    	//Log.i( "Quake2", "check_updates");
    	
    	// do not check for updates in debug mode
    	if (debug)
    		return;
    	
    	// wait a little
    	SystemClock.sleep(2000);   	   	
    	
    	String url = "http://sites.google.com/site/quake2android/downloads"
    		+ "/check-" + version +".txt";
    	
    	Log.i( "Quake2", "checking " + url);
    	
    	String content = "ERROR";
    	try {    		
    	 	 BufferedReader buf = new BufferedReader(new InputStreamReader(
        			 new URL(url).openStream() ), 1024);
    	 	 
    	 	 content = buf.readLine();
    	 	 
		} catch (Exception e) {
			//e.printStackTrace();
		}
		
		Log.i( "Quake2", "content=" + content);
		
		if ( "UPDATE".equals(content)){
							
			if (mRenderer.state==mRenderer.STATE_RESET)  {   
				
				 Message.obtain(handlerUI, MSG_SHOW_DIALOG, DIALOG_CHECK_UPDATE, 0)  			   
					.sendToTarget();
			}
			
		} 
 	
    }

    
    public void show_tools() {
        
    	setContentView(R.layout.tools);
    	

    	CharSequence[] mirrors;

    	Resources res = getResources();
    	mirrors = res.getTextArray(R.array.mirrors); 
    	
    	Spinner s = (Spinner) findViewById(R.id.spinner);
        ArrayAdapter adapter = ArrayAdapter.createFromResource(
                this, R.array.mirrors, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s.setAdapter(adapter);

		
		Button button_download = (Button)this.findViewById(R.id.button_download);
		button_download.setOnClickListener( new OnClickListener() {  
        	             //@Override  
        	             public void onClick(View v) {  
        	            	 
        	            	 Spinner s = (Spinner) findViewById(R.id.spinner);
        	            	 CharSequence url = (CharSequence)s.getSelectedItem();
        	            	 
        	            	 Log.i( "Quake2", "url= " + url);
        	            	
        	            	 
        	            	 mDownloadTask = new DownloadTask();
        	            	 mDownloadTask.set_context(Quake2.this, url.toString());
        	            	 mDownloadTask.execute();
        	            	 
        	             }  
        }); 
		
		final Context context = this;
		
		Button button_config = (Button)this.findViewById(R.id.button_config);
		button_config.setOnClickListener( new OnClickListener() {  
        	             //@Override  
        	             public void onClick(View v) {  
        	            	
        	         
        	            	copy_asset( true, "config.cfg" );						
        	            	copy_asset( true, "overlay1.tga" );
        	            	copy_asset( true, "overlay2.tga" );
        	            	
        	            	Toast.makeText( context, "configuration restored",  Toast.LENGTH_SHORT)
        	            			.show();
        	             }  
        }); 
    }
    
    public void copy_asset( boolean overwrite, String name )
    {  	
    	if (overwrite ||
    		!(new File("/sdcard/baseq2/"+name)).exists()){
    		copy_asset( name,
    					"/sdcard/baseq2/"+name);
    	}
    }
    
    public void copy_asset( String name_in, String name_out )
    {
    	Log.i( "Quake2.java", String.format("copy_asset %s to %s",
    			name_in, name_out));
    	
    	AssetManager assets = this.getAssets();
    	
		try {
			InputStream in = assets.open(name_in);
			OutputStream out = new FileOutputStream(name_out);
			
			copy_stream(in, out);			
	    	
	    	out.close();
	    	in.close();
	    	
		} catch (Exception e) {

			e.printStackTrace();
		}
    	
    	
    	

    }
    
    public static void copy_stream( InputStream in, OutputStream out )
    throws IOException
    {
    	byte[] buf = new byte[1024];    	
    	while(true){
    		int count = in.read(buf);
    		if (count<=0) break;
			out.write(buf, 0, count);
    	}
    }
    
    public void show_keyboard() {
	 	// show soft keyboard
    	/*
		InputMethodManager inputManager = (InputMethodManager)
			this.getSystemService(Context.INPUT_METHOD_SERVICE); 
		
		inputManager.toggleSoftInput(  InputMethodManager.SHOW_FORCED , 
				0);
		*/
    	
    	// show overlay keyboard
    	overlay = 2;
	
    }
    

	

    
    public void start_quake2() {

    	// check PAK file
    	if (!(new File("/sdcard/baseq2/pak0.pak")).exists()
    			&& !(new File("/sdcard/baseq2/pics/colormap.pcx")).exists()) {
    		showDialog(DIALOG_PAK_NOT_FOUND);
    		return;
    	}

    	// check CFG file   
    	// if not present, copy it silently
    	copy_asset( false, "config.cfg" );
    	copy_asset( false, "overlay1.tga" );
    	copy_asset( false, "overlay2.tga" );

    	showDialog(DIALOG_LOADING);


    	mRenderer.speed_limit = enable_ecomode ? 40 : 0;



    	vibrator = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);


    	// Create our Preview view and set it as the content of our
    	// Activity
    	mGLSurfaceView = new QuakeView(this);
    	//mGLSurfaceView.setGLWrapper( new MyWrapper());
    	//mGLSurfaceView.setDebugFlags(GLSurfaceView.DEBUG_CHECK_GL_ERROR | GLSurfaceView.DEBUG_LOG_GL_CALLS);
    	//setEGLConfigChooser  (int redSize, int greenSize, int blueSize, int alphaSize, int depthSize, int stencilSize)
    	//mGLSurfaceView.setEGLConfigChooser(8,8,8,0,16,0);
    	mGLSurfaceView.setEGLConfigChooser( new QuakeEGLConfigChooser() );



    	mGLSurfaceView.setRenderer(mRenderer);

    	// This will keep the screen on, while your view is visible. 
    	mGLSurfaceView.setKeepScreenOn(true);

    	setContentView(mGLSurfaceView);
    	mGLSurfaceView.requestFocus();
    	mGLSurfaceView.setFocusableInTouchMode(true);

    	if (enable_sensor){
    		SensorManager sensorMgr = (SensorManager) getSystemService(SENSOR_SERVICE);

    		Sensor sensor = sensorMgr.getDefaultSensor(
    				//Sensor.TYPE_ORIENTATION);
    				Sensor.TYPE_ACCELEROMETER);
    			
    		Log.i( "Quake2.java", "using sensor "
    				+ (sensor!=null ? sensor.getName() : "null") );

    		// SENSOR_DELAY_GAME => 200 ms
    		sensorMgr.registerListener(this, sensor, 
    				SensorManager.SENSOR_DELAY_GAME );
    		//SensorManager.SENSOR_DELAY_NORMAL );
    	}


    }
	
	class QuakeEGLConfigChooser implements EGLConfigChooser {
    
    /*
     need the good config for hardware acceleration
     
     on HTC G2 (RGB 565 screen) default EGLConfigChooser is ok
     on Samsung Galaxy (RGB 888 screen) default EGLConfigChooser is NOT hardware accelerated
     
     
     GL strings when hardware accelerated :
     
    D/libquake2.so( 9520): GL_VENDOR: QUALCOMM, Inc.
    D/libquake2.so( 9520): GL_RENDERER: Q3Dimension MSM7500 01.02.08 0 4.0.0
    D/libquake2.so( 9520): GL_VERSION: OpenGL ES 1.0-CM
    D/libquake2.so( 9520): GL_EXTENSIONS: GL_ARB_texture_env_combine GL_ARB_texture_env_crossbar GL_ARB_texture_env_dot3 GL_ARB_texture_mirrored_repeat GL_ARB_vertex_buffer_object GL_ATI_extended_texture_coordinate_data_formats GL_ATI_imageon_misc GL_ATI_texture_compression_atitc GL_EXT_blend_equation_separate GL_EXT_blend_func_separate GL_EXT_blend_minmax GL_EXT_blend_subtract GL_EXT_stencil_wrap GL_OES_byte_coordinates GL_OES_compressed_paletted_texture GL_OES_draw_texture GL_OES_fixed_point GL_OES_matrix_palette GL_OES_point_size_array GL_OES_point_sprite GL_OES_read_format GL_OES_single_precision GL_OES_vertex_buffer_object GL_QUALCOMM_vertex_buffer_object GL_QUALCOMM_direct_texture  EXT_texture_env_add

    all configs available :


I/Quake2.java(11435): numConfigs=22
I/Quake2.java(11435): config= EGLConfig rgba=5650 depth=16 stencil=0 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5551 depth=16 stencil=0 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=4444 depth=16 stencil=0 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5650 depth=16 stencil=4 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5551 depth=16 stencil=4 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=4444 depth=16 stencil=4 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5650 depth=0 stencil=0 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5551 depth=0 stencil=0 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=4444 depth=0 stencil=0 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5650 depth=0 stencil=4 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5551 depth=0 stencil=4 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=4444 depth=0 stencil=4 native=0 buffer=16 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=8888 depth=16 stencil=0 native=0 buffer=32 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=8888 depth=16 stencil=4 native=0 buffer=32 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=8888 depth=0 stencil=0 native=0 buffer=32 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=8888 depth=0 stencil=4 native=0 buffer=32 caveat=0x3038
I/Quake2.java(11435): config= EGLConfig rgba=5650 depth=0 stencil=0 native=1 buffer=16 caveat=0x3050
I/Quake2.java(11435): config= EGLConfig rgba=5650 depth=16 stencil=0 native=1 buffer=16 caveat=0x3050
I/Quake2.java(11435): config= EGLConfig rgba=8888 depth=0 stencil=0 native=1 buffer=32 caveat=0x3050
I/Quake2.java(11435): config= EGLConfig rgba=8888 depth=16 stencil=0 native=1 buffer=32 caveat=0x3050
I/Quake2.java(11435): config= EGLConfig rgba=0008 depth=0 stencil=0 native=1 buffer=8 caveat=0x3050
I/Quake2.java(11435): config= EGLConfig rgba=0008 depth=16 stencil=0 native=1 buffer=8 caveat=0x3050



   */
    
	
    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
    	
    	Log.i( "Quake2.java", "chooseConfig");
    	
    	
    	int[] mConfigSpec  = {
            //EGL10.EGL_RED_SIZE, 8,
            //EGL10.EGL_GREEN_SIZE, 8,
            //EGL10.EGL_BLUE_SIZE, 8,
            //EGL10.EGL_ALPHA_SIZE, 0,
            EGL10.EGL_DEPTH_SIZE, 16,
            //EGL10.EGL_STENCIL_SIZE, 0,
            EGL10.EGL_NONE};
    	
   
    	
        int[] num_config = new int[1];
        egl.eglChooseConfig(display, mConfigSpec, null, 0, num_config);

        int numConfigs = num_config[0];
        
    	Log.i( "Quake2.java", "numConfigs="+numConfigs);

        if (numConfigs <= 0) {
            throw new IllegalArgumentException(
                    "No EGL configs match configSpec");
        }

        EGLConfig[] configs = new EGLConfig[numConfigs];
        egl.eglChooseConfig(display, mConfigSpec, configs, numConfigs,
                num_config);
        
        
        if (debug)
	        for(EGLConfig config : configs) {
	        	Log.i( "Quake2.java", "found EGL config : " + printConfig(egl,display,config));       	
	        }
        
        
        // best choice : select first config
        Log.i( "Quake2.java", "selected EGL config : " + printConfig(egl,display,configs[0]));
        
        return configs[0];
  

    
    }
    

    private  String printConfig(EGL10 egl, EGLDisplay display,
            EGLConfig config) {
    	
    	 int r = findConfigAttrib(egl, display, config,
                 EGL10.EGL_RED_SIZE, 0);
         int g = findConfigAttrib(egl, display, config,
                  EGL10.EGL_GREEN_SIZE, 0);
         int b = findConfigAttrib(egl, display, config,
                   EGL10.EGL_BLUE_SIZE, 0);
         int a = findConfigAttrib(egl, display, config,
                 EGL10.EGL_ALPHA_SIZE, 0);
         int d = findConfigAttrib(egl, display, config,
                 EGL10.EGL_DEPTH_SIZE, 0);
         int s = findConfigAttrib(egl, display, config,
                 EGL10.EGL_STENCIL_SIZE, 0);

             /*
              * 
              * EGL_CONFIG_CAVEAT value 
              
         #define EGL_NONE		       0x3038	
         #define EGL_SLOW_CONFIG		       0x3050	
         #define EGL_NON_CONFORMANT_CONFIG      0x3051	
*/
         
         return String.format("EGLConfig rgba=%d%d%d%d depth=%d stencil=%d", r,g,b,a,d,s)
        		+ " native=" + findConfigAttrib(egl, display, config, EGL10.EGL_NATIVE_RENDERABLE, 0)
         	    + " buffer=" + findConfigAttrib(egl, display, config, EGL10.EGL_BUFFER_SIZE, 0)
         		+ String.format(" caveat=0x%04x" , findConfigAttrib(egl, display, config, EGL10.EGL_CONFIG_CAVEAT, 0))
         		;
    	
     
         
         
    }
    
    private int findConfigAttrib(EGL10 egl, EGLDisplay display,
            EGLConfig config, int attribute, int defaultValue) {
    	
    	int[] mValue = new int[1];
        if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
            return mValue[0];
        }
        return defaultValue;
    }
    
	} // end of QuakeEGLConfigChooser
	
    //protected void onStart();
    

    

    @Override
    protected void onPause() {
    	
    	Log.i( "Quake2.java", "onPause" );
         
        super.onPause();
        
        please_exit = true;
        
       	if (mDownloadTask!=null)
       		mDownloadTask.please_abort = true; // stop download     	
       	
       	save_preferences();
       	
        if (mRenderer.state!=mRenderer.STATE_RESET){        	
        	
        	
        	mGLSurfaceView.queueEvent(new Runnable(){
        		 public void run() {
        			 
        			mGLSurfaceView.onPause();
        			        		    	
        			// I/Quake2.java(13899): Quake2Quit
        			// D/libquake2.so(13899): R_Shutdown
        			// E/libEGL  (13899): call to OpenGL ES API with no current context (logged once per thread)
			    	sQuake2Quit();
			    	
			       	System.exit(0); // kill process will force reload library on next launch

        		 }});
      	
        	
    	}
    }

    @Override
    protected void onResume() {
    	
    	Log.i( "Quake2.java", "onResume" );
    	
        super.onResume();
        if (mRenderer.state!=mRenderer.STATE_RESET){
        	mGLSurfaceView.onResume();
        }
    }

    @Override
    protected void onRestart() {
    	
    	Log.i( "Quake2.java", "onRestart" );
         
        super.onRestart();
        
      
    }
    

    
    
    @Override
    protected void onStop() {
    	
    	Log.i( "Quake2.java", "onStop" );
         
        super.onStop();
      

       

    	

    }
    
    @Override
    protected void onDestroy() {
    	
    	Log.i( "Quake2.java", "onDestroy" );
         
        super.onDestroy();
        
      
    }
    
 

    /* Creates the menu items */
    @Override
    //public boolean onCreateOptionsMenu(Menu menu) {
    public boolean onPrepareOptionsMenu(Menu menu) {
    	
    	menu.clear();
        menu.add("About");
        
        if (mRenderer.state==mRenderer.STATE_RESET){  
        menu.add("Main");
        menu.add("Tools");
        menu.add("Settings");       
        }
        else {
        menu.add("Keyboard");
        }
        
        menu.add("Exit");

        
        return true;
    }

    
    /* Handles item selections */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
    	
    	/*System.out.println("onOptionsItemSelected "+item
    			+ " id=" +item.getItemId() 
    			+ " group=" + item.getGroupId()
    			);
    	*/
    	
    	if ("About".equals(item.toString())){
    		showDialog(DIALOG_ABOUT_ID);
    		return true;
    	}
    	
    	if ("Main".equals(item.toString())){           
    		show_main();  
    		return true;
    	}
    	
    	if ("Exit".equals(item.toString())){
    		showDialog(DIALOG_EXIT_ID);
     		return true;
    	}
    	
    	if ("Tools".equals(item.toString())){           
            show_tools();    
    		return true;
    	}
    	
    	if ("Settings".equals(item.toString())){           
    		show_settings();    
    		return true;
    	}
    	
    	if ("Keyboard".equals(item.toString())){           
            show_keyboard();    
    		return true;
    	}
    	
   
     
        return false;
    }
    
    static final int DIALOG_EXIT_ID = 0,
					    DIALOG_ABOUT_ID = 1,
					    DIALOG_PAK_NOT_FOUND = 2,
					    DIALOG_ERROR = 3,
					    DIALOG_LOADING = 4,
					    DIALOG_CHECK_UPDATE = 5;
    
    @Override
    protected Dialog onCreateDialog(int id) {
    	AlertDialog.Builder builder = new AlertDialog.Builder(this);
 		
        switch(id) {
        case DIALOG_EXIT_ID:
        	builder.setMessage("Are you sure you want to exit?")
    		       .setCancelable(false)
    		       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
    		           public void onClick(DialogInterface dialog, int id) {
    		                finish();
    		           }
    		       })
    		       .setNegativeButton("No", new DialogInterface.OnClickListener() {
    		           public void onClick(DialogInterface dialog, int id) {
    		                dialog.cancel();
    		           }
    		       });
    		return builder.create();
    		
        case DIALOG_ABOUT_ID:
  
        	builder.setMessage("Quake2 on Android\n\n"
        			+"Credits :\n\n"
        			+" Julien Eyriès for initial port, 3D, audio\n\n"
        			+" Guillaume Legris for adding controls\n\n"
        			+" Lukács Péter for the icon\n\n"
        			+" Id Software for Quake2 GPL code\n\n"   
        			+" People at icculus.org for linux patch\n\n"   	
        			+" Olli Hinkka for NanoGL\n\n"
        			)
    		       ;
    		return builder.create();

        case DIALOG_PAK_NOT_FOUND:
        	builder.setMessage("Sorry, pak0.pak file not found.\n"
        	                  +"Press Menu then Tools for automated download\n"
        	                  +"or download manually (see homepage)"
        						)
    		       ;
    		return builder.create();

        case DIALOG_ERROR:
        	builder.setMessage(error_message)
    		       .setCancelable(false)
    		       .setPositiveButton("Exit", new DialogInterface.OnClickListener() {
    		           public void onClick(DialogInterface dialog, int id) {
    		                finish();
    		           }
    		       })
    		       ;
    		return builder.create();
    		
        case DIALOG_LOADING: 
            pd_loading = new ProgressDialog(this);
            pd_loading.setMessage("loading ...");
            pd_loading.setIndeterminate(true);
            pd_loading.setCancelable(false);
            pd_loading.setOnDismissListener( new OnDismissListener(){

				//@Override
				public void onDismiss(DialogInterface arg0) {
					// restore focus 
					mGLSurfaceView.requestFocus();

				}
            });
            return pd_loading;

        case DIALOG_CHECK_UPDATE:
        	builder.setMessage("An update is available for Quake 2.\n"
        			+"Do you want to launch the Browser to install the update ?")
		       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
		           public void onClick(DialogInterface dialog, int id) {
		        	   startActivity(new Intent(Intent.ACTION_VIEW, 
		        			   Uri.parse("http://sites.google.com/site/quake2android"))); 
		           }
		       })
		       .setNegativeButton("No", new DialogInterface.OnClickListener() {
		           public void onClick(DialogInterface dialog, int id) {
		                dialog.cancel();
		           }
		       });
    		return builder.create();
        }
        return null;
    }
    
  

    class QuakeView extends GLSurfaceView {
    	
    	  /*--------------------
         * Event handling
         *--------------------*/

        /**
         * Key states (read by the native side). DO NOT rename without modifying the
         * native side.
         */
        private static final int MAX_KEY_EVENTS = 128;
        public int[] keyEvents = new int[MAX_KEY_EVENTS];
        public int eventId = 0;

        /*
         *  Key codes understood by the Quake engine.
         */
        
    

        static final int K_TAB = 9, K_ENTER = 13, K_ESCAPE = 27, K_SPACE = 32, K_BACKSPACE = 127, K_UPARROW = 128, K_DOWNARROW = 129,
                K_LEFTARROW = 130, K_RIGHTARROW = 131, K_ALT = 132, K_CTRL = 133, K_SHIFT = 134, K_F1 = 135, K_F2 = 136, K_F3 = 137,
                K_F4 = 138, K_F5 = 139, K_F6 = 140, K_F7 = 141, K_F8 = 142, K_F9 = 143, K_F10 = 144, K_F11 = 145, K_F12 = 146, K_INS = 147,
                K_DEL = 148, K_PGDN = 149, K_PGUP = 150, K_HOME = 151, K_END = 152, K_KP_HOME = 160, K_KP_UPARROW = 161, K_KP_PGUP = 162,
                K_KP_LEFTARROW = 163, K_KP_5 = 164, K_KP_RIGHTARROW = 165, K_KP_END = 166, K_KP_DOWNARROW = 167, K_KP_PGDN = 168,
                K_KP_ENTER = 169, K_KP_INS = 170, K_KP_DEL = 171, K_KP_SLASH = 172, K_KP_MINUS = 173, K_KP_PLUS = 174, K_MOUSE1 = 200,
                K_MOUSE2 = 201, K_MOUSE3 = 202, K_MOUSE4 = 241, K_MOUSE5 = 242, K_JOY1 = 203, K_JOY2 = 204, K_JOY3 = 205, K_JOY4 = 206,
                K_AUX1 = 207, K_AUX2 = 208, K_AUX3 = 209, K_AUX4 = 210, K_AUX5 = 211, K_AUX6 = 212, K_AUX7 = 213, K_AUX8 = 214, K_AUX9 = 215,
                K_AUX10 = 216, K_AUX11 = 217, K_AUX12 = 218, K_AUX13 = 219, K_AUX14 = 220, K_AUX15 = 221, K_AUX16 = 222, K_AUX17 = 223,
                K_AUX18 = 224, K_AUX19 = 225, K_AUX20 = 226, K_AUX21 = 227, K_AUX22 = 228, K_AUX23 = 229, K_AUX24 = 230, K_AUX25 = 231,
                K_AUX26 = 232, K_AUX27 = 233, K_AUX28 = 234, K_AUX29 = 235, K_AUX30 = 236, K_AUX31 = 237, K_AUX32 = 238, K_MWHEELDOWN = 239,
                K_MWHEELUP = 240, K_PAUSE = 255, K_LAST = 256;
      
         
        public QuakeView(Context context) {
            super(context);
            
            init_buttons();
        }

        public void postKeyEvent( int key, int down )
        {
        	synchronized (keyEvents) {
                if (eventId < keyEvents.length)
                	keyEvents[eventId++] = key | down<<8;
            }
        }
        
        public boolean onKeyDown(int keyCode, KeyEvent event) {

        	int code = convertCode(keyCode, event);
        	
        	if (debug)
        		Log.i("Quake2.java", "onKeyDown: " + keyCode + " code: " + code);

            if (code > 0 && code < K_LAST) {
            	postKeyEvent( code, 1 );
            	return true;
            }

            return false;
        }

        public boolean onKeyUp(int keyCode, KeyEvent event) {

        	int code = convertCode(keyCode, event);
        	
        	if (debug)
        		Log.i("Quake2.java", "onKeyUp: " + keyCode + " code: " + code);

        	if (code > 0 && code < K_LAST) {
        		postKeyEvent( code, 0 );
        		return true;
            }
        	
            return false;
        }

        private int convertCode(int keyCode, KeyEvent event) {
            int code = 0;

        
            
                switch (keyCode) {
                //case KeyEvent.KEYCODE_J:
                case KeyEvent.KEYCODE_DPAD_LEFT:
                    code = K_LEFTARROW;
                    break;
                //case KeyEvent.KEYCODE_K:
                case KeyEvent.KEYCODE_DPAD_RIGHT:
                    code = K_RIGHTARROW;
                    break;
                //case KeyEvent.KEYCODE_I:
                case KeyEvent.KEYCODE_DPAD_UP:
                    code = K_UPARROW;
                    break;
                //case KeyEvent.KEYCODE_M:
                case KeyEvent.KEYCODE_DPAD_DOWN:
                    code = K_DOWNARROW;
                    break;
                case KeyEvent.KEYCODE_DPAD_CENTER:
                case KeyEvent.KEYCODE_ENTER:
                    code = K_ENTER;
                    break;
                case KeyEvent.KEYCODE_TAB:
                    code = K_TAB;
                    break;
                case KeyEvent.KEYCODE_BACK:
                    code = K_ESCAPE;
                    break;
                case KeyEvent.KEYCODE_DEL:
                    //code = K_DEL;
                    code = K_BACKSPACE;
                    break;
                case KeyEvent.KEYCODE_SHIFT_LEFT:
                case KeyEvent.KEYCODE_SHIFT_RIGHT:
                    code = K_SHIFT;
                    break;
                case KeyEvent.KEYCODE_ALT_LEFT:
                case KeyEvent.KEYCODE_ALT_RIGHT:
                    code = K_ALT;
                    break;
                case KeyEvent.KEYCODE_STAR:
                    code = '*';
                    break;
                case KeyEvent.KEYCODE_PLUS:
                    code = K_KP_PLUS;
                    break;
                case KeyEvent.KEYCODE_MINUS:
                    code = K_KP_MINUS;
                    break;
                case KeyEvent.KEYCODE_SLASH:
                    code = K_KP_SLASH;
                    break;
                }
            
                if (code==0){
                	
                    // normal keys should be passed as lowercased ascii
                    
                	code = event.getUnicodeChar();
                	
                	if ( code<32 || code>127)
                		code = 0;
                	
                	if ( code >= 'A' && code <= 'Z')
                		code = code - 'A' + 'a';
                	
                	/*
                    if ((keyCode >= KeyEvent.KEYCODE_A) && (keyCode <= KeyEvent.KEYCODE_Z)) {
                        code = keyCode - KeyEvent.KEYCODE_A + 'a';
                    } else if ((keyCode >= KeyEvent.KEYCODE_0) && (keyCode <= KeyEvent.KEYCODE_9)) {
                        code = keyCode - KeyEvent.KEYCODE_0 + '0';
                    } 
                    */
                    
                }
            return code;
        }

 
        // removed : maybe ask guillaume ..
        //public boolean onTrackballEvent(final MotionEvent e) 
            
        

        private class QuakeButton {
        	float x0,y0,x1,y1;
        	int code;
        	int state;
        	long timestamp;
        	
        	public QuakeButton( float x, float y, float w, float h, int code ){
        		this.x0 = x;
        		this.y0 = y;
        		this.x1 = x+w;
        		this.y1 = y+h;
        		this.code = code;
        		this.state = 0;
        		this.timestamp = 0;
        	}
        	
        	public boolean check( float tx, float ty){
        		return ( tx>x0 && tx<x1
        				&& ty>y0 && ty<y1);      		
        	}
        	
        	public void move( float x, float y ){
        		
        	}
        	
        	public void press( float x, float y ){
        		
        		this.timestamp = SystemClock.uptimeMillis();
        		
	        	if (enable_vibrator)
	    			performHapticFeedback(0, //HapticFeedbackConstants.VIRTUAL_KEY,
	    				HapticFeedbackConstants.FLAG_IGNORE_GLOBAL_SETTING );
	    		
        		
        		if ( this.state == 0){
        			this.state = 1;

        			postKeyEvent( this.code, 1 );
        		} else {
        			// release button
        			this.state = 0;     	
        			postKeyEvent(  this.code, 0 );
        		}
        		
        		
        	}
        	
    		
        	public void release( float x, float y ){
        		
        			
        		long downtime = SystemClock.uptimeMillis() - this.timestamp;
        		
        		if (downtime < 1000){
        			// short press
        			this.state = 0;     	
        			postKeyEvent(  this.code, 0 );
        		
        		} else {
        			// long press
        			// => lock button
        		}
        	}
        }
        
        

        private List<QuakeButton> overlay1, overlay2 ;
        private QuakeButton current_button = null; // no button down
                
        public QuakeButton check_buttons( List<QuakeButton> buttons, float tx, float ty )
        {
        	for (QuakeButton button : buttons)
        	{
        		if ( button.check(tx,ty) )
        			return button;
        	}
        	return null;
        }
        
        public void init_buttons()
        {
        	overlay1 = new ArrayList<QuakeButton>();
        	 
        	 float a,b,c;
        	 
        	 a = 1/3.0f;
        	 
        	 
        	 // KEYBOARD
        	 overlay1.add( new QuakeButton(   0,   0, a, a, 0 ){
        		 @Override
        		 public void press( float x, float y ){
        			 overlay = (overlay + 1)% 3;
        		 }
        		 @Override
        		 public void release( float x, float y ){
        			 
        		 }
        	 });
        	 
        	 // CALIBRATE
        	 overlay1.add(  new QuakeButton(   a,   0, a, a, 0 ){
        		 @Override
        		 public void press( float x, float y ){
        			 synchronized (sensorEvents) {
                      	pitch_ref = pitch;   	
                      }
        		 }
        		 @Override
        		 public void release( float x, float y ){
        			 
        		 }
        	 });
        	 
        	 // AUX3
        	 overlay1.add( new QuakeButton( 2*a,   0, a, a, K_AUX3 ));
        	 
        	 // AUX6
        	 overlay1.add(  new QuakeButton( 2*a,   a, a, a, K_AUX6 ));
        	 
        	 // AUX9
        	 overlay1.add(  new QuakeButton( 2*a, 2*a, a, a, K_AUX9 ));

        	 // CONTROL VIEW
        	 overlay1.add( new QuakeButton(   0,   a, 2*a, 2*a, 0 ){
        		 @Override
        		 public void press( float x, float y ){
        			 synchronized (sensorEvents) {
                     	touch_state = 1;
                     	touch_xref = x;
                     	touch_yref = y;
                     	touch_x = x;
                     	touch_y = y;    	
                     }
        		 }
        		 @Override
        		 public void move( float x, float y ){
        			synchronized (sensorEvents) {
             			touch_x = x;
                     	touch_y = y;
 	            	}	 
        		 }
        		 
        		 @Override
        		 public void release( float x, float y ){
 	            	synchronized (sensorEvents) {
	            		touch_state = 0;
	            	}	 
        		 }
        	 });
        	 
        	 // KEYBOARD
        	 overlay2 = new ArrayList<QuakeButton>();
        	 
        	 a = 1/10.0f;
        	 b = 1/6.0f;
        	 

        	
        	 overlay2.add( new QuakeButton( 0*a, 0*b, a, b, K_F1 ));
        	 overlay2.add( new QuakeButton( 1*a, 0*b, a, b, K_F2 ));
        	 overlay2.add( new QuakeButton( 2*a, 0*b, a, b, K_F3 ));
        	 overlay2.add( new QuakeButton( 3*a, 0*b, a, b, K_F4 ));
        	 
        	 overlay2.add( new QuakeButton( 5*a, 0*b, a, b, '0' ));
        	 overlay2.add( new QuakeButton( 6*a, 0*b, a, b, '1' ));
        	 overlay2.add( new QuakeButton( 7*a, 0*b, a, b, '2' ));
        	 overlay2.add( new QuakeButton( 8*a, 0*b, a, b, '3' ));
        	 overlay2.add( new QuakeButton( 9*a, 0*b, a, b, '4' ));
        	 
        	 overlay2.add( new QuakeButton( 5*a, 1*b, a, b, '5' ));
        	 overlay2.add( new QuakeButton( 6*a, 1*b, a, b, '6' ));
        	 overlay2.add( new QuakeButton( 7*a, 1*b, a, b, '7' ));
        	 overlay2.add( new QuakeButton( 8*a, 1*b, a, b, '8' ));
        	 overlay2.add( new QuakeButton( 9*a, 1*b, a, b, '9' ));
        	 
        	 overlay2.add( new QuakeButton( 0*a, 2*b, a, b, 'a' ));
        	 overlay2.add( new QuakeButton( 1*a, 2*b, a, b, 'z' ));
        	 overlay2.add( new QuakeButton( 2*a, 2*b, a, b, 'e' ));
        	 overlay2.add( new QuakeButton( 3*a, 2*b, a, b, 'r' ));
        	 overlay2.add( new QuakeButton( 4*a, 2*b, a, b, 't' ));
        	 overlay2.add( new QuakeButton( 5*a, 2*b, a, b, 'y' ));
        	 overlay2.add( new QuakeButton( 6*a, 2*b, a, b, 'u' ));
        	 overlay2.add( new QuakeButton( 7*a, 2*b, a, b, 'i' ));
        	 overlay2.add( new QuakeButton( 8*a, 2*b, a, b, 'o' ));
        	 overlay2.add( new QuakeButton( 9*a, 2*b, a, b, 'p' ));
        	 
        	 overlay2.add( new QuakeButton( 0*a, 3*b, a, b, 'q' ));
        	 overlay2.add( new QuakeButton( 1*a, 3*b, a, b, 's' ));
        	 overlay2.add( new QuakeButton( 2*a, 3*b, a, b, 'd' ));
        	 overlay2.add( new QuakeButton( 3*a, 3*b, a, b, 'f' ));
        	 overlay2.add( new QuakeButton( 4*a, 3*b, a, b, 'g' ));
        	 overlay2.add( new QuakeButton( 5*a, 3*b, a, b, 'h' ));
        	 overlay2.add( new QuakeButton( 6*a, 3*b, a, b, 'j' ));
        	 overlay2.add( new QuakeButton( 7*a, 3*b, a, b, 'k' ));
        	 overlay2.add( new QuakeButton( 8*a, 3*b, a, b, 'l' ));
        	 overlay2.add( new QuakeButton( 9*a, 3*b, a, b, 'm' ));
        	 
        	 overlay2.add( new QuakeButton( 1.5f*a, 4*b, a, b, 'w' ));
        	 overlay2.add( new QuakeButton( 3.5f*a, 4*b, a, b, 'x' ));
        	 overlay2.add( new QuakeButton( 4.5f*a, 4*b, a, b, 'c' ));
        	 overlay2.add( new QuakeButton( 5.5f*a, 4*b, a, b, 'v' ));
        	 overlay2.add( new QuakeButton( 6.5f*a, 4*b, a, b, 'b' ));
        	 overlay2.add( new QuakeButton( 7.5f*a, 4*b, a, b, 'n' ));
        	         	 
        	 overlay2.add( new QuakeButton(      0, 4*b, 1.5f*a, b, K_SHIFT ));       	 
        	 overlay2.add( new QuakeButton( 8.5f*a, 4*b, 1.5f*a, b, K_BACKSPACE ));
        	 
        	 overlay2.add( new QuakeButton( 3.5f*a, 5*b, 3.0f*a, b, K_SPACE ));
        	 overlay2.add( new QuakeButton( 6.5f*a, 5*b, 1.5f*a, b, '.' ));

        	 overlay2.add( new QuakeButton( 8.0f*a, 5*b, 2.0f*a, b, 0 ){
        		 @Override
        		 public void press( float x, float y ){
        			 overlay = 0;
        		 }
        		 @Override
        		 public void release( float x, float y ){
        			 
        		 }
        	 });        
        }
        
        
        
        
        public boolean onTouchEvent(final MotionEvent e) {
        	
            //Log.i("Quake2.java", "onTouchEvent:" +  e.getX() + " " + e.getY());
            
        	float x = e.getX() / this.getWidth();
        	float y = e.getY() / this.getHeight();
        	

            switch (e.getAction()) {          

            case MotionEvent.ACTION_DOWN:
            	
            	List buttons = overlay1;
            	if (overlay==2) buttons = overlay2;
            	current_button = check_buttons(buttons,x,y);           	
            	if ( current_button!=null )            		
            		current_button.press(x, y);
            	
                break;
                
            case MotionEvent.ACTION_MOVE:
            	           	
            	if ( current_button!=null )            		
            		current_button.move(x, y);
            	
            	break;
                              
            case MotionEvent.ACTION_UP:
            	
            	if ( current_button!=null )            		
            		current_button.release(x, y);
            	current_button = null;
            	
            	break;
            	
            }
            
            return true;
        }
        
 
        

        
        public void kbdUpdate() {
            //Log.i("Quake2.java", "kbdUpdate");
            synchronized (keyEvents) {
            	
            
            	
               
                for (int k=0;k<eventId;k++){
                	
                	int event = keyEvents[k];
                	           
                	int key = event & 0x00ff;
                	int down = (event>>8) & 0x00ff;
	                
                	if (debug) Log.i("Quake2.java", 
                			String.format("keyEvent %d %d" ,key, down ));
                	
                	 Quake2KeyEvent(  key, down );
                
                }


                // Clear the event buffer size
                eventId = 0;
            }

        }
    }  // end of QuakeView
    
    ///////////// GLSurfaceView.Renderer implementation ///////////
    
    class QuakeRenderer implements GLSurfaceView.Renderer {
    	
    
    private static final int 
    	STATE_RESET=0,
    	STATE_SURFACE_CREATED=1,
    	STATE_RUNNING=2,
    	STATE_ERROR=100;
    	
    

	private int state = STATE_RESET; 
	

	// deprecated ... use setEGLConfigChooser
    //public int[] getConfigSpec() {

     
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    	
    	Log.d("Renderer", "onSurfaceCreated");
    	
    	switch(state){
        
        case STATE_RESET:
       	 state=STATE_SURFACE_CREATED;
       	 break;
       	        	 
        default:
       	 throw new Error("wrong state");
        
        }


    	
    	//this.gl = gl;
    	
    	//CHECK THIS:
    	//AndroidRenderer.renderer.set_gl(gl);
    	//AndroidRenderer.renderer.set_size(320,240);
    	
        /*
         * By default, OpenGL enables features that improve quality
         * but reduce performance. One might want to tweak that
         * especially on software renderer.
         */
        gl.glDisable(GL10.GL_DITHER);

        /*
         * Some one-time OpenGL initialization can be made here
         * probably based on features of this particular context
         */
         gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT,
                 //GL10.GL_FASTEST);
        		 GL10.GL_NICEST);
         
    }
    

    	
   
    
    private void init( int width, int height ){
    	


         
           
           Log.i( "Quake2", "version : " + Quake2GetVersion());
           
           Log.i( "Quake2", "screen size : " + width + "x"+ height);
           
           Quake2SetWidth( width );
           Quake2SetHeight( height );

    	
    	/*
    	Log.d("Renderer", "init");
    	for (int k=0;k<60;k++){
    		Log.d("Renderer", "sleep "+k);
	    	try {
	 		   Thread.sleep(1000);
	 		} catch (InterruptedException e) {
	 			e.printStackTrace();
	 		}
    	}
 		*/
 		
         ////////////////
       

       Log.i("Quake2", "Quake2Init start");
    	
       int ret = Quake2Init();
       
       Log.i("Quake2", "Quake2Init done");
    	
       if (ret!=0){
    	   
    	   error_message = String.format("initialisation error detected (code %d)\nworkaround : reinstall APK or reboot phone.", ret) ;
    	   Log.e( "Quake2", error_message   );
    	 
    	   //System.exit(1);
    	   
    	   state = STATE_ERROR;
    	   // error, wrong thread ...
    	   //showDialog(DIALOG_ERROR);
    	   
 
    	   Message.obtain(handlerUI, MSG_SHOW_DIALOG, DIALOG_ERROR, 0 )  			   
    	   			.sendToTarget();
    
    	   
    	   return;
       }
       
  
       
       
           tstart = SystemClock.uptimeMillis();
    }
    

   
    

      
    
  	
  	private int counter_fps=0;
  	private long tprint_fps= 0;
  	private int framenum=0;
  	
  	// speed limit : 10 FPS
  	private int speed_limit = 0; 
  								//40;
  								//100;
  								//200;
  	
  	private int vibration_duration = //0;
  							100;
  	
    private boolean vibration_running = false;
    private long vibration_end;
    
    private long tprev = 0;
    
    //// new Renderer interface
	public void onDrawFrame(GL10 gl) {
		
		
		switch(state){
        
        case STATE_RUNNING:
       	 // nothing
       	 break;
       	 
        case STATE_ERROR:
        	{
        	long s = SystemClock.uptimeMillis();
        	
            gl.glClearColor(((s>>10)&1)*1.0f,((s>>11)&1)*1.0f,((s>>12)&1)*1.0f,1.0f);
            
            gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
            
            gl.glFinish();
        	}
            return;
       	        	 
        default:
       	 throw new Error("wrong state");
        
        }
    	
		long tnow = SystemClock.uptimeMillis(); 
		int tdelta = (int)(tnow-tprev) ;
		if ( tprev == 0 ) tdelta = 0;
		tprev = tnow;
		
		if ( timelimit!=0 && (tnow-tstart)>= timelimit){
			Log.i( "Quake2.java", "Timer expired. exiting");
			finish();
			timelimit = 0;
		}
		
		// compute FPS
        if ( (tnow-tprint_fps) >= 1000){
        	if (debug)
        		Log.i("Quake2",String.format( "FPS= %d",counter_fps));
        	
        	tprint_fps = tnow;
        	counter_fps = 0;        	
        }
        counter_fps ++;
        

    	
        // dissmiss loading dialog after some time
		if (pd_loading!=null){
			if (Quake2GetDisableScreen()==0){
			pd_loading.dismiss();
			pd_loading = null;
			// restore focus _ NOT HERE ...
			//mGLSurfaceView.requestFocus();
			
		     // start audio thread
		       if (enable_audio)
			       new Thread( new Runnable(){
						public void run() {
							try {
								audio_thread();
							} catch (IOException e) {
								e.printStackTrace();
							}		
						}}).start();
			}
		}
    	
    	
    	/*
         * Usually, the first thing one might want to do is to clear
         * the screen. The most efficient way of doing this is to use
         * glClear().
         */
       
		int vibration = 0;
		
		Quake2SetOverlay(overlay);
			
		//Log.i("Quake2", "Quake2Frame start");
		
		//if (framenum < 30)
		//	Log.i("Quake2", String.format("frame %d",framenum));

    	mGLSurfaceView.kbdUpdate();
    	   	
    	moveUpdate( tdelta );
    	
    	while( sQuake2Frame()==0 );   	
        
        framenum ++;
        
        if (enable_vibrator)
        	vibration = Quake2GetVibration();

    
        //Log.i("Quake2", "Quake2Frame done");
		
        
		long tafter = SystemClock.uptimeMillis(); 
		

        
        if (vibration_running
        		&& (tafter -vibration_end)> 0)
        	vibration_running = false;    
        
        if (!vibration_running
        		&& vibration == 1 
        		&& vibration_duration > 0 ){
	    	// Start the vibration
	    	vibrator.vibrate(vibration_duration);
	    	vibration_running = true;
	    	vibration_end = tafter + vibration_duration;
	    }
        
 

		// speed limit : 10 FPS
        // probably a bad idea, because Android will try to run 
        // other processes in the background if we go to sleep ..
   
        if (speed_limit>0){
	        long tsleep =  speed_limit - (tafter - tnow);
			if ( tsleep > 0 )
				SystemClock.sleep( tsleep );
        }
        


        
    }
	



	public void onSurfaceChanged(GL10 gl, int width, int height) {

    	
    	Log.d("Renderer", "onSurfaceChanged");
    	

        //AndroidRenderer.renderer.set_gl(gl);
        //AndroidRenderer.renderer.set_size(width,height);
    	
         gl.glViewport(0, 0, width, height);
         
         switch(state){
         
         case STATE_SURFACE_CREATED:
        	 init( width, height );
        	 state=STATE_RUNNING;
        	 break;
        	 
         case STATE_RUNNING:
        	 //nothing
        	 break;
        	 
         default:
        	 throw new Error("wrong state");
         }


         /*
          * Set our projection matrix. This doesn't have to be done
          * each time we draw, but usually a new projection needs to
          * be set when the viewport is resized.
          */
/*
         float ratio = (float) width / height;
         gl.glMatrixMode(GL10.GL_PROJECTION);
         gl.glLoadIdentity();
         gl.glFrustumf(-ratio, ratio, -1, 1, 1, 10);
         */
    }


    } // end of QuakeRenderer
    
    ///////////////// JNI methods /////////////////////////////
    
    /* this is used to load the 'quake2' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.HelloJni/lib/liquake2.so at
     * installation time by the package manager.
     */
	
    static {
       
           //System.loadLibrary("quake2");
           
           try {
               Log.i("JNI", "Trying to load libquake2.so");
               System.loadLibrary("quake2");
           }
           catch (UnsatisfiedLinkError ule) {
               Log.e("JNI", "WARNING: Could not load libquake2.so");
           }
    }

    /* A native method that is implemented by the
     * 'quake2' native library, which is packaged
     * with this application.
     */
    
    // synchronized access
    
    private static Object quake2Lock = new Object();
    
    private static int sQuake2Init(){
    	int ret;
	    synchronized(quake2Lock) { 	
	    	ret = Quake2Init();
	    }
	    return ret;
    }
    
    private static int sQuake2Frame(){
    	int ret;
	    synchronized(quake2Lock) { 	
	    	ret = Quake2Frame();
	    }
	    return ret;
    }
    
    private static int sQuake2Quit(){
    	int ret;
	    synchronized(quake2Lock) { 	
	    	Log.i( "Quake2.java", "Quake2Quit" );
	    	ret = Quake2Quit();
	    }
	    return ret;
    }
    
    private static int sQuake2PaintAudio( ByteBuffer buf ){
    	int ret;
	    synchronized(quake2Lock) { 	
	    	ret = Quake2PaintAudio(buf);
	    }
	    return ret;
    }

  
    
    // raw acces
    
    private static native String Quake2GetVersion();
    
    private static native int Quake2Init();
    
    private static native int Quake2Frame();
    
    private static native int Quake2Quit();
    
    private static native int Quake2Test();
    
    private static native void Quake2SetWidth( int value );
    
    private static native void Quake2SetHeight( int value );
    
    private static native void Quake2SetOverlay( int value );
    
    private static native int Quake2PaintAudio( ByteBuffer buf );

    private static native int Quake2GetDisableScreen();
    
    private static native int Quake2GetVibration();
         
    private static native void Quake2KeyEvent(  int key, int down );
    
    private static native void Quake2MoveEvent( int mode, 
			int forwardmove, int sidemove, int upmove,
			float pitch, float yaw, float roll );
    
    
    
    
    /*----------------------------
     * Audio
     *----------------------------*/
    
    public void audio_thread() throws IOException{

    	
    	int audioSize = (2048*4); 
    	
    	ByteBuffer audioBuffer = ByteBuffer.allocateDirect(audioSize);
    	
    	byte[] audioData = new byte[audioSize];
    	
    	
    	// output to a PCM file
    	// adb pull /sdcard/quake2.pcm .
    	// sox -L -s -2 -c 2 -r 44100 -t raw quake2.pcm -t wav quake2.wav
        //FileOutputStream out = new FileOutputStream( new File( "/sdcard/quake2.pcm") );

    	
	    AudioTrack oTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 22050, //44100,
	                                       AudioFormat.CHANNEL_CONFIGURATION_STEREO,
	                                       AudioFormat.ENCODING_PCM_16BIT, 
	                                       4*(22050/5), // 200 millisecond buffer
	                                       				// => impact on audio latency
	                                       AudioTrack.MODE_STREAM);
	
	    Log.i("Quake2", "start audio");
	 
	    // Start playing data that is written
	    oTrack.play();
	
	    long tstart = SystemClock.uptimeMillis();
	    
	    while (!please_exit){
	    	
	    	long tnow = SystemClock.uptimeMillis() ;
	    	
	    	// timelimit
	    	if ( timelimit!=0 && (tnow-tstart) > timelimit)
	    		break;
	    	
	    		    		
	    	sQuake2PaintAudio( audioBuffer );          	    
	    	

	        audioBuffer.position(0);
	        audioBuffer.get(audioData);
	        

	    	
		    // Write the byte array to the track
		    oTrack.write(audioData, 0, audioData.length);    


	        
	    }
	    
	    Log.i("Quake2", "stop audio");
	    
	    // Done writting to the track
	    oTrack.stop();	    
	    
	}


    
    /*----------------------------
     * Sensors
     *----------------------------*/
    
	//@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// do nothing
		
	}

	
	
	private long timestamp = 0;
	private Object sensorEvents = new Object();
	private float pitch = PITCH_DEFAULT, roll = 0.0f;
	private float pitch_ref = PITCH_DEFAULT;
	private int touch_state = 0;
	private float touch_x, touch_y;
	private float touch_xref, touch_yref ;
	
	//@Override
	public void onSensorChanged(SensorEvent event) {
	
		// milliseconds
		float delta = (float)(event.timestamp - timestamp) * 1e-6f;
		timestamp = event.timestamp;
		
		/*
		if (debug)
			Log.i("Quake2", String.format("onSensorChanged %s %d %.0f %.0f %.0f %.0f", 
					event.sensor.getName(), event.accuracy,
					event.values[0], event.values[1], event.values[2],
					delta));	
		*/
		
		
		  
		if ( event.sensor.getType() == Sensor.TYPE_ACCELEROMETER ){

			float[] gravity = new float[3];
			float[] geomagnetic = new float[3];
			float[] R = new float[9];
			float[] inR = new float[9];
			float[] outR = new float[9];
			float[] I = new float[9];
			float[] values = new float[3];

			gravity[0] = event.values[0];
			gravity[1] = event.values[1];
			gravity[2] = event.values[2];

			geomagnetic[0] = 0.0f;
			geomagnetic[1] = 1.0f;  // avoid getRotationMatrix failure
			geomagnetic[2] = 0.0f;

			boolean result;

			result = SensorManager.getRotationMatrix( inR, I, gravity, geomagnetic);
			if (!result)
				Log.w("Quake2", "getRotationMatrix failure");

			
			result = SensorManager.remapCoordinateSystem(inR, SensorManager.AXIS_X, SensorManager.AXIS_Y, R);
			if (!result)
				Log.w("Quake2", "remapCoordinateSystem failure");
			 
			SensorManager.getOrientation( R, values);

			// to degrees
			values[0] *= 180.0 / Math.PI;
			values[1] *= 180.0 / Math.PI;
			values[2] *= 180.0 / Math.PI;
		

/*
			if (debug)
				Log.i("Quake2", String.format("orientation %.0f %.0f %.0f", 
						values[0], values[1], values[2]));	
	*/		
			synchronized (sensorEvents) {

				pitch = values[2];
				roll = -values[1];
				
				if (invert_roll)
					roll = -roll;
			}
			

			
		}

		else if ( event.sensor.getType() == Sensor.TYPE_ORIENTATION ){

		
			synchronized (sensorEvents) {

			
				pitch = -event.values[2];
				roll = -event.values[1];
				
				if (invert_roll)
					roll = -roll;
				
				
			}
		
		}
		
		// landscape mode
		// pitch = rotation autour des X
		// roll = rotation autour des Y
		// flat => pitch 0, roll 0

		/*
		if (debug)
			Log.i("Quake2", String.format("onSensorChanged pitch= %.0f roll= %.0f", 
					pitch, roll));	
*/
		
	}
    
	/*
	 Quake 2 angle definition :
	 
	// angle indexes
	#define	PITCH				0		// up / down
	#define	YAW					1		// left / right
	#define	ROLL				2		// fall over


	*/
	

	private static final int
		MOVE_NOTHING = 0,
		MOVE_FORWARDMOVE = 1,
		MOVE_YAW = 2,
		MOVE_VIEW = 3;
		
	private int move_state = MOVE_NOTHING;

	
	
	public static final float PITCH_DEFAULT = -50.0f;
	private float qpitch = 0.0f; // set	

	
	
	public void moveUpdate( int msec ) {
		
		
		int mode = 2; // set pitch
		int forwardmove = 0;
		float qyaw = 0.0f; // delta
	
		float p = 0.0f;
		float r = 0.0f;
		float tx = 0.0f;
		float ty = 0.0f;
		
		synchronized (sensorEvents) {
			
			/*
			if (debug)
				Log.i("Quake2", String.format("pitch= %.0f pitch_ref= %.0f roll= %.0ff", 
						pitch, pitch_ref, roll));
			*/
			
			// get info from sensor
			if (enable_sensor) {
				 p = pitch - pitch_ref;
				 r = roll ; // always 0 degree as roll ref
			}
			
			// get info from touchscreen
			if (touch_state==1){
				
				// compute roll from touchscreen X
				tx = 180.0f * (touch_x - touch_xref);
				touch_xref = touch_x; // delta
			
				// compute pitch from touchscreen Y
				ty = 180.0f * (touch_y - touch_yref);
				//touch_yref = touch_y;
	
			} 

			/*
			if (debug)
				Log.i("Quake2", String.format("p= %.0f r= %.0f ts= %d tx= %.0f ty= %.0f", 
						p, r, touch_state, tx, ty));
			*/
			
			float ANGLE_MIN = 5.0f;
			
			// remove small pitch
			if ( Math.abs(p) < ANGLE_MIN )
				p = 0.0f;
			else 
				p -= Math.signum(p) * ANGLE_MIN;
			
			// remove small roll
			if (Math.abs(r) < ANGLE_MIN)
				r = 0.0f;
			else 
				r -= Math.signum(r) * ANGLE_MIN;
			
	
			
			// state change 
			
			switch ( move_state ){
			
			case MOVE_NOTHING:
				if ( touch_state == 1)
					move_state = MOVE_VIEW;
				else if ( p != 0  )
					move_state = MOVE_FORWARDMOVE;
				else if ( r != 0 )
					move_state = MOVE_YAW;
				
				break;
				
			case MOVE_FORWARDMOVE:
				if ( touch_state == 1)
					move_state = MOVE_VIEW;
				else if ( p == 0 )
					move_state = MOVE_NOTHING;
				else if ( Math.abs(r) > 5.0f ) // urgent yaw
					move_state = MOVE_YAW;
				
				break;
				
			case MOVE_YAW:
				if ( touch_state == 1)
					move_state = MOVE_VIEW;
				else if ( r == 0 )
					move_state = MOVE_NOTHING;
				else if ( Math.abs(p) > 5.0f ) // urgent forwardmove
					move_state = MOVE_FORWARDMOVE;
				
				break;
			
			case MOVE_VIEW:
				if ( touch_state == 0)
					move_state = MOVE_NOTHING;
				break;
				
			}

		
		} // end of synchronized (sensorEvents)
		
		if ( move_state == MOVE_NOTHING ){

			// release slowly qpitch
			
			if  ( qpitch!=0 ) {
				// 180 degree in 1 seconds
				float delta = msec * (180.0f/1000);
								
				if ( qpitch > 1.0f){
					qpitch -= delta;
					if (qpitch<0) qpitch = 0;
				} else if ( qpitch < -1.0f){
					qpitch += delta;
					if (qpitch> 0) qpitch = 0;
				} else
					qpitch = 0;
	
			}
			
			// no move
			//qpitch = 0.0f; // too fast
			qyaw = 0.0f;
			forwardmove = 0;
		}
		
		if ( move_state == MOVE_FORWARDMOVE ){
							
			
			// precise move until 10 degree then faster
			if ( Math.abs(p) < 10.0f )
				forwardmove = (int)((100.0f/10.0f) * p);
			else
				forwardmove = (int)((200.0f/10.0f) * (p-Math.signum(p)*10.0f)
									+ Math.signum(p)* 100.0f );
				
		
			// full speed in 20 degrees
			//forwardmove = (int)((300.0f/20.0f) * p);
					
			// limit max forward speed
			if ( forwardmove > 300)
				forwardmove = 300;
			else if ( forwardmove < -300)
				forwardmove = -300;
			
			// no pitch when moving
			qpitch = 0.0f;
			qyaw = 0.0f;
		
		}
		
		if ( move_state == MOVE_YAW ){
			
			// precise yaw until 10 degree then faster
			if ( Math.abs(r) < 10.0f )
				qyaw = -0.5f * r;
			else
				qyaw = -1.0f * (r-Math.signum(r)*10.0f) + Math.signum(r)*-5.0f;
			
			//qyaw = -1.0f * r;
			
			if (qyaw > 20.0f)
				qyaw = 20.0f ;			
			else if (qyaw < -20.0f)
				qyaw = -20.0f;	
			
			// no pitch when moving
			qpitch = 0.0f;
			forwardmove = 0;
		}

		
		if ( move_state == MOVE_VIEW)
		{
			qyaw = -tx ;
			qpitch = ty ;
			
			// no move when viewing
			forwardmove = 0;
		}	
		
		/*
		if (debug)
			Log.i("Quake2", String.format("moveEvent state= %d forwardmove= %d qpitch= %.0f qyaw= %.0f", 
				move_state, forwardmove, qpitch, qyaw));	
		*/
		
		
		
		Quake2MoveEvent( mode, 
				forwardmove, 0, 0,
				qpitch,  qyaw, 0 );
		
	} // end of viewUpdate()
	
	
}


