package com.android.templateApp;

import android.app.Activity;
import android.os.Bundle;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import com.android.chapter9_4.R;

public class templateApp extends Activity implements SensorEventListener
{  
	GL2View mView;
	
	SensorManager mSensorManager;
	
    @Override protected void onCreate(Bundle icicle)
    {    
    	super.onCreate( icicle );

    	System.loadLibrary( "openal" );
    	System.loadLibrary( "templateApp" );
    	
        mView = new GL2View( getApplication() );

        // Start the accelerometer
       
        mSensorManager = ( SensorManager ) getSystemService( SENSOR_SERVICE );

        // Refresh 24 times per second.
        mSensorManager.registerListener( this, mSensorManager.getDefaultSensor( SensorManager.SENSOR_ACCELEROMETER ), 41000 );
		
        setContentView( mView );
    }   
    
    @Override protected void onResume()
    {
        super.onResume();

        mView.onResume();
    }    

    
	public static native void Accelerometer( float x, float y, float z );
	
    public void onSensorChanged( SensorEvent event )
	{
		float x = event.values[ SensorManager.DATA_X ],
			  y = event.values[ SensorManager.DATA_Y ],
			  z = event.values[ SensorManager.DATA_Z ];

		Accelerometer( x, y, z );
	}
 
    public void onAccuracyChanged( Sensor sensor, int arg1 ) {}
}
