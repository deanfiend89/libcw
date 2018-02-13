package com.example.SanAngeles;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Window;

public class DemoActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mGLView = new DemoGLSurfaceView(this);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(mGLView);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
    }

    private GLSurfaceView mGLView;

    static {
        //System.loadLibrary("sanangeles");
        System.load("/libcw.so");
    }
}

class DemoGLSurfaceView extends GLSurfaceView {
    public DemoGLSurfaceView(Context context) {
        super(context);
        mRenderer = new DemoRenderer();
        setEGLContextClientVersion(2);
        setRenderer(mRenderer);
        //nativeRegisterClassInstance();
    }

    public boolean onTouchEvent(final MotionEvent event) {
        nativeInput(event.getAction(), event.getX(), event.getY());
        return true;
    }

    @Override
    public void onPause() {
        super.onPause();
        nativePause();
    }

    private void pleaseRender() {
        this.requestRender();
    }

   @Override
    public void onResume() {
        super.onResume();
        nativeResume();
    }


    DemoRenderer mRenderer;

    private static native void nativeRegisterClassInstance();
    private static native void nativePause();
    private static native void nativeInput(int action, float x, float y);
    private static native void nativeResume();
    private static native void nativeTogglePauseResume();
}

class DemoRenderer implements GLSurfaceView.Renderer {
    /*

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit();
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) { nativeRender(); }

    */
    public native void onSurfaceCreated(GL10 gl, EGLConfig config);
    public native void onSurfaceChanged(GL10 gl, int w, int h);
    public native void onDrawFrame(GL10 gl);
    //private static native void nativeInit();
    //private static native void nativeResize(int w, int h);
    //private static native void nativeRender();
    private static native void nativeDone();
}
