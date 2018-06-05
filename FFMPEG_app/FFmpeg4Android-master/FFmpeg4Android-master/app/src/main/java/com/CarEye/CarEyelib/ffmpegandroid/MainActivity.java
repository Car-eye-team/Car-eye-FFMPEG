package com.CarEye.CarEyelib.ffmpegandroid;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

public class MainActivity extends Activity {
    String TAG = "Car-eye-ffmpeg";
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //TestFliter();
        TestEncode();
    }
    void TestFliter(){
        Log.d("ffmpeg4android", "OSD init start: ");
        new Thread(new Runnable() {
            @Override
            public void run() {
                int loop = 0;
                FileOutputStream out;
                FileInputStream in;
                FFmpegNative ffmpeg = new FFmpegNative();
                ffmpeg.FFMPEG_init();
                long ret = ffmpeg.InitOSD(1280, 720, 10, 10, 28, 0x00ff00, String.format("/mnt/sdcard/arial.ttf"), String.format("ddddd"));
                if (ret == 0) {
                    Log.d(TAG, "OSD init fail: "+ret);
                }
                Log.d(TAG, "OSD blend ");
                byte[] data=new byte[1280*720*3/2];
                try {
                    File f = new File("/mnt/sdcard/out.yuv");
                    if(f.exists()) f.delete();
                    f.createNewFile();
                    out = new FileOutputStream(f);
                    File input = new File("/mnt/sdcard/input.yuv");
                    in = new FileInputStream(input);
                    int len;
                    while(loop<10)
                    {
                        loop++;
                        if(in.read(data,0,1280*720*3/2)<0)
                        {
                            Log.d(TAG, "read fail:");
                            break;
                        }else {
                            String txt = "car-eye-filter:" + new SimpleDateFormat("yyyy-MM-dd").format(new Date());
                            int result =  ffmpeg.blendOSD(ret,data, txt);
                            out.write(data,0,1280*720*3/2);
                            Log.d(TAG, "write data sucessful:"+result);
                        }
                    }
                    in.close();
                    out.close();
                    ffmpeg.DelOSD(ret);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
    void TestEncode() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                int loop = 0;
                FileOutputStream out;
                FileInputStream in;
                FFmpegNative ffmpeg = new FFmpegNative();
                EncodeParamInfo info = new EncodeParamInfo();
                info.fps = 25;
                info.width = 1280;
                info.height = 720;
                info.InVedioType = 0;
                info.OutVedioType = 0x1C;
                info.InputAuidoType = 0;
                info.VideoBitrate = 3000000;
                ffmpeg.FFMPEG_init();
                long handle = ffmpeg.InitEncode(info);
                if (handle == 0) {
                    Log.d(TAG, "init encoder fail");
                    return;
                }
                Log.d(TAG, "init encoder success");
                byte[] data = new byte[1280 * 720 * 3 / 2];
                byte[] out_data = new byte[1280 * 720 * 3 / 2];
                try {
                    File f = new File("/mnt/sdcard/out.h264");
                    if (f.exists()) f.delete();
                    f.createNewFile();
                    out = new FileOutputStream(f);
                    File input = new File("/mnt/sdcard/input.yuv");
                    in = new FileInputStream(input);
                    int len;
                    while (loop<10) {
                        if (in.read(data, 0, 1280 * 720 * 3 / 2) < 0) {
                            Log.d(TAG, "read fail:");
                            break;
                        } else {
                            int result = ffmpeg.EncodeData(handle, 0, loop++, data, out_data);
                            if (result > 0) {
                                out.write(out_data, 0, result);
                                Log.d(TAG, "encoder sucessful:"+result);
                            }else {
                                Log.d(TAG, "encoder fail:"+result);
                            }
                        }
                    }
                    in.close();
                    out.close();
                    ffmpeg.DestroyEncode(handle);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
    void TestDecode() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                int loop = 0;
                FileOutputStream out;
                FileInputStream in;
                FFmpegNative ffmpeg = new FFmpegNative();
                DecoderParamInfo frame_info = new DecoderParamInfo();
                frame_info.FramesPerSecond = 25;
                frame_info.ACodec = 0x15002;
                frame_info.SampleRate = 44100;
                frame_info.Channels = 2;
                frame_info.BitsPerSample = 16;
                frame_info.AudioBitrate = 64000;
                ffmpeg.FFMPEG_init();
                long handle = ffmpeg.InitDecode(frame_info);
                if (handle == 0) {
                    Log.d(TAG, "init encoder fail");
                    return;
                }
                Log.d(TAG, "init encoder success");
                byte[] data = new byte[1280 * 720 * 3 / 2];
                byte[] out_data = new byte[1280 * 720 * 3 / 2];
                try {
                    File f = new File("/mnt/sdcard/out.h264");
                    if (f.exists()) f.delete();
                    f.createNewFile();
                    out = new FileOutputStream(f);
                    File input = new File("/mnt/sdcard/input.yuv");
                    in = new FileInputStream(input);
                    int len;
                    while (loop<10) {
                        if (in.read(data, 0, 1280 * 720 * 3 / 2) < 0) {
                            Log.d(TAG, "read fail:");
                            break;
                        } else {
                            int result = ffmpeg.DecodeData(handle, 0, data, out_data);
                            if (result > 0) {
                                out.write(out_data, 0, result);
                                Log.d(TAG, "encoder sucessful:"+result);
                            }else {
                                Log.d(TAG, "encoder fail:"+result);
                            }
                        }
                    }
                    in.close();
                    out.close();
                    ffmpeg.DestroyDecode(handle);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
}
