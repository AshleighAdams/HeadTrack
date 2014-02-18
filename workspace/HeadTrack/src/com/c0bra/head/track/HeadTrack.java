package com.c0bra.head.track;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.R.array;
import android.R.bool;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.Matrix;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.StrictMode;
import android.os.StrictMode.ThreadPolicy;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.view.Menu;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.ToggleButton;

@TargetApi(9) public class HeadTrack extends Activity implements SensorEventListener {
	boolean lastOn = false;
	PowerManager.WakeLock wl = null;
	
	private SensorManager mSensorManager = null;
	private final float[] mAcceleration = new float[3];
    private final ExponentialFilter mAccelerationFilter = new ExponentialFilter(0.5F, 3);
    private float mDeclination;
    private boolean mHasDeclination;
    private boolean mHasRotationVectorSensor;
    private final float[] mMagneticField = new float[3];
    private final ExponentialFilter mMagneticFieldFilter = new ExponentialFilter(0.5F, 3);
    private final float[] mRemappedRotationMatrix = new float[16];
    private final float[] mRotationMatrix = new float[16];
    private final float[] mTempMatrix = new float[16];
    private final float[] mOrientation = new float[3];
	 
    // angular speeds from gyro
    private float[] gyro = new float[3];
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_head_track);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "HeadTrack");
        
        mSensorManager = (SensorManager) this.getSystemService(SENSOR_SERVICE);
        initListeners();
    }
    
    public void initListeners(){
    	mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_FASTEST); // 16000
    	mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_FASTEST);
    	mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD), SensorManager.SENSOR_DELAY_FASTEST);
    }
    
    public static float radiansToDegrees(float paramFloat)
    {
      return 57.29578F * paramFloat;
    }

    public static float radiansToNormalizedTilt(float paramFloat)
    {
      return 0.5F - 0.3183099F * paramFloat;
    }
    
    private void updateOrientation()
    {
    	float[] arrayOfFloat = this.mRotationMatrix;
        if (this.mHasDeclination)
          Matrix.rotateM(arrayOfFloat, 0, this.mDeclination, 0.0F, 0.0F, 1.0F);
        Matrix.rotateM(arrayOfFloat, 0, 90.0F, 1.0F, 0.0F, 0.0F);
        
        System.arraycopy(this.mRotationMatrix, 0, this.mTempMatrix, 0, this.mTempMatrix.length);
        Matrix.rotateM(this.mTempMatrix, 0, -90.0F, 1.0F, 0.0F, 0.0F);
        //SensorManager.remapCoordinateSystem(this.mTempMatrix, 1, 3, this.mTempMatrix);
        SensorManager.getOrientation(this.mTempMatrix, mOrientation);
        
        float[] tmp = new float[3];
        tmp[0] = radiansToDegrees(this.mOrientation[0]);
        tmp[1] = radiansToDegrees(this.mOrientation[1]);
        tmp[2] = radiansToDegrees(this.mOrientation[2]);
        
        sendData((byte)1, toByta(tmp));
    }
    
    public void onSensorChanged(SensorEvent event) {
        switch(event.sensor.getType())
        {
        case Sensor.TYPE_ACCELEROMETER:
        	mAccelerationFilter.filter(this.mAcceleration, event.values);
        	break;
        case Sensor.TYPE_MAGNETIC_FIELD:
        	mMagneticFieldFilter.filter(this.mMagneticField, event.values);
        	//updateOrientation();
        	break;
        case Sensor.TYPE_ROTATION_VECTOR:
        	SensorManager.getRotationMatrixFromVector(this.mRotationMatrix, event.values);
            updateOrientation();
        	break;
        }
    }
        
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_head_track, menu);
        return true;
    }
    
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
		
	}
	
	
	public void sendData(byte opcode, byte[] datain)
	{
		ToggleButton tggl = (ToggleButton)this.findViewById(R.id.tbbSendData);
		
		boolean checked = tggl.isChecked();
		
		if(checked != lastOn)
		{
			lastOn = checked;
			if(checked)
				wl.acquire();
			else
				wl.release();
		}
		
		if(!tggl.isChecked()) return;
		
		EditText server = (EditText)this.findViewById(R.id.tbServer);
    	String host = server.getText().toString();
    	
    	byte[] data = new byte[datain.length + 1];
    	data[0] = opcode;
    	
    	System.arraycopy(datain, 0, data, 1, datain.length);
    	
    	InetAddress addr;
		try {
			addr = InetAddress.getAllByName(host)[0];
		} catch (UnknownHostException e) {
			tggl.setChecked(false);
			return;
		}
    	
    	DatagramPacket packet = new DatagramPacket(data, data.length, addr, 61);
    	DatagramSocket dsocket = null;
		try 
		{
			ThreadPolicy tp = ThreadPolicy.LAX;
			StrictMode.setThreadPolicy(tp);
			
			dsocket = new DatagramSocket();
			dsocket.send(packet);
			
		} 
		catch (SocketException e) {} 
		catch (IOException e) {}
		
		if(dsocket != null)
			dsocket.close();
	
	}
	public static byte[] toByta(int data) {
	    return new byte[] {
	        (byte)((data >> 24) & 0xff),
	        (byte)((data >> 16) & 0xff),
	        (byte)((data >> 8) & 0xff),
	        (byte)((data >> 0) & 0xff),
	    };
	}
	public static byte[] toByta(float data) {
	    return toByta(Float.floatToRawIntBits(data));
	}
	public static byte[] toByta(float[] data) {
	    if (data == null) return null;
	    // ----------
	    byte[] byts = new byte[data.length * 4];
	    for (int i = 0; i < data.length; i++)
	        System.arraycopy(toByta(data[i]), 0, byts, i * 4, 4);
	    return byts;
	}

	class ExponentialFilter
	{
	  private final float mAlpha;
	  private final int mExponent;

	  public ExponentialFilter(float paramFloat, int paramInt)
	  {
	    if (paramInt < 1)
	      throw new IllegalArgumentException("exponent must be >= 1");
	    if ((paramFloat < 0.0F) || (paramFloat > 1.0F))
	      throw new IllegalArgumentException("alpha must be in [0 .. 1]");
	    this.mAlpha = paramFloat;
	    this.mExponent = paramInt;
	  }

	  public void filter(float[] paramArrayOfFloat1, float[] paramArrayOfFloat2)
	  {
	    if (paramArrayOfFloat2.length != paramArrayOfFloat1.length)
	      throw new IllegalArgumentException("length mismatch");
	    for (int i = 0; i < paramArrayOfFloat2.length; i++)
	    {
	      float f1 = paramArrayOfFloat2[i] - paramArrayOfFloat1[i];
	      float f2 = Math.abs(f1);
	      if (f2 < 1.0F)
	        for (int j = 1; j < this.mExponent; j++)
	          f1 *= f2;
	      paramArrayOfFloat1[i] += f1 * this.mAlpha;
	    }
	  }
	}
}
