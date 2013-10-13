package com.android.templateApp;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import com.android.chapter9_4.R;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

class GL2View extends GLSurfaceView implements SurfaceHolder.Callback
{
	public Renderer r;
	
	public GL2View( Context context )
	{
        super( context );

        setEGLContextFactory( new ContextFactory() );

        				  // new ConfigChooser( 8, 8, 8, 8, 1, 1 )
        setEGLConfigChooser( new ConfigChooser( 5, 6, 5, 0, 1, 1 ) );

        r = new Renderer();
        
        setRenderer( r );
        
        r.apkFilePath = context.getPackageResourcePath();
	}

    private static class ContextFactory implements GLSurfaceView.EGLContextFactory
    {
    	public EGLContext createContext( EGL10 egl, EGLDisplay display, EGLConfig eglConfig )
    	{
    		int[] attrib_list = { 0x3098,
    							  2,
    							  EGL10.EGL_NONE };
            
    		EGLContext context = egl.eglCreateContext( display,
    												   eglConfig,
    												   EGL10.EGL_NO_CONTEXT,
    												   attrib_list );
    		return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context)
        { egl.eglDestroyContext( display, context ); }
    }
       
    public static native void ToucheBegan( float x, float y, int tap_count );

    public static native void ToucheMoved( float x, float y, int tap_count );
    
    public static native void ToucheEnded( float x, float y, int tap_count );
    
    private long last_tap = 0;
    
    private int tap_count = 1;
    
    public boolean onTouchEvent( final MotionEvent event )
    {
        switch( event.getAction() )
        {
	        case MotionEvent.ACTION_DOWN:
	        {
	        	if( event.getEventTime() - last_tap < 333 ) tap_count = tap_count + 1;
	        	else tap_count = 1;

	        	last_tap = event.getEventTime();

	        	ToucheBegan( event.getX(0), event.getY(0), tap_count );
	        	break;
	        }
	        
	        case MotionEvent.ACTION_MOVE:
	        {
	        	ToucheMoved( event.getX(0), event.getY(0), tap_count );
	        	break;
	        }
	        
	        case MotionEvent.ACTION_UP:
	        {
	        	ToucheEnded( event.getX(0), event.getY(0), tap_count );
	        	break;
	        }
        }

        return true;
    }       
   
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {

        protected int mRedSize,
        			  mGreenSize,
        			  mBlueSize,
        			  mAlphaSize,
        			  mDepthSize,
        			  mStencilSize;
        
        public ConfigChooser( int r, int g, int b, int a, int depth, int stencil )
        {
            mRedSize     = r;
            mGreenSize   = g;
            mBlueSize    = b;
            mAlphaSize   = a;
            mDepthSize   = depth;
            mStencilSize = stencil;
        }

        
        private static int[] s_configAttribs =
        {
            EGL10.EGL_RED_SIZE		 , 5,
            EGL10.EGL_GREEN_SIZE	 , 6,
            EGL10.EGL_BLUE_SIZE		 , 5,
            EGL10.EGL_ALPHA_SIZE	 , 0,
            EGL10.EGL_RENDERABLE_TYPE, 4,
            EGL10.EGL_NONE
        };

        
        public EGLConfig chooseConfig( EGL10 egl, EGLDisplay display )
        {
            int[] num_config = new int[ 1 ];

            egl.eglChooseConfig( display,
            					 s_configAttribs,
            					 null,
            					 0,
            					 num_config );

            int numConfigs = num_config[ 0 ];

            EGLConfig[] configs = new EGLConfig[ numConfigs ];
            
            egl.eglChooseConfig( display,
            					 s_configAttribs,
            					 configs,
            					 numConfigs,
            					 num_config );
       
            return chooseConfig( egl, display, configs );
        }
        
        
        public EGLConfig chooseConfig( EGL10 egl, EGLDisplay display, EGLConfig[] configs )
        {
            for( EGLConfig config : configs )
            {
                int depth   = findConfigAttrib( egl, display, config, EGL10.EGL_DEPTH_SIZE  , 0 ),
                    stencil = findConfigAttrib( egl, display, config, EGL10.EGL_STENCIL_SIZE, 0 );

                if( depth < mDepthSize || stencil < mStencilSize ) continue;

                int r = findConfigAttrib( egl, display, config, EGL10.EGL_RED_SIZE  , 0 ),
                   	g =	findConfigAttrib( egl, display, config, EGL10.EGL_GREEN_SIZE, 0 ),
                   	b =	findConfigAttrib( egl, display, config, EGL10.EGL_BLUE_SIZE , 0 ),
                    a =	findConfigAttrib( egl, display, config, EGL10.EGL_ALPHA_SIZE, 0 );

                if( r == mRedSize   && 
                	g == mGreenSize && 
                	b == mBlueSize  && 
                	a == mAlphaSize ) return config;
            }
            
            return null;
        }
        
        
        private int findConfigAttrib( EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue )
        {
        	int[] mValue = new int[ 1 ];
        	
            if( egl.eglGetConfigAttrib( display,
            							config,
            							attribute,
            							mValue ) ) return mValue[ 0 ];
            return defaultValue;
        }
     } 
    
    public static native void Init( int w, int h, String apkFilePath );
     
    public static native void Draw();
     
    public static class Renderer implements GLSurfaceView.Renderer
    {
    	public String apkFilePath;
    	public int	  width;
    	public int	  height;
    	
    	public void onDrawFrame( GL10 gl ){ Draw(); }

    	private char init_once = 0;
    	
        public void onSurfaceChanged( GL10 gl, int width, int height )
        {
        	if( init_once == 0 )
        	{
        		this.width  = width;
        		this.height = height;
        		
         		Init( width, height, apkFilePath );
	        	init_once = 1;
        	}
        }
       
        public void onSurfaceCreated( GL10 gl, EGLConfig config ) { }
    }
}
