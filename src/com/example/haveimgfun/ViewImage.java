package com.example.haveimgfun;




import java.io.File;
import java.io.FileOutputStream;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class ViewImage extends Activity {
	// Declare Variable
	TextView text;
	ImageView imageview;
	ImageView imageview2;
	ImageView imageview3;
	Button btnNDK, btnRestore; 
	Bitmap bmp;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Get the view from view_image.xml
		setContentView(R.layout.view_image);
		
	

		// Retrieve data from MainActivity on GridView item click
		Intent i = getIntent();

		// Get the position
		int position = i.getExtras().getInt("position");

		// Get String arrays FilePathStrings
		String[] filepath = i.getStringArrayExtra("filepath");

		// Get String arrays FileNameStrings
		String[] filename = i.getStringArrayExtra("filename");

		// Locate the TextView in view_image.xml
		text = (TextView) findViewById(R.id.imagetext);

		// Load the text into the TextView followed by the position
		text.setText(filename[position]);

		// Locate the ImageView in view_image.xml
		imageview = (ImageView) findViewById(R.id.full_image_view1);
		imageview2 = (ImageView) findViewById(R.id.full_image_view2);
		imageview3 = (ImageView) findViewById(R.id.full_image_view3);

		// Decode the filepath with BitmapFactory followed by the position
		bmp = BitmapFactory.decodeFile(filepath[position]);

		// Set the decoded bitmap into ImageView
		imageview.setImageBitmap(bmp);
		
		btnRestore = (Button) this.findViewById(R.id.btnRestore);  
        btnRestore.setOnClickListener(new ClickEvent());  
        btnNDK = (Button) this.findViewById(R.id.btnNDK);  
        btnNDK.setOnClickListener(new ClickEvent());  
	        
    }
	
	class ClickEvent implements View.OnClickListener {  
		public void onClick(View v) {  
			if (v == btnNDK) {  
                long current = System.currentTimeMillis();  
                
                //Grayscale
                //Bitmap bmpgray = toGrayscale (bmp);
                
                //Save Bitmap image
                /*
                String root = Environment.getExternalStorageDirectory().toString();
                File myDir = new File(root + "/saved_images");    
                myDir.mkdirs();
                String fname = "ImageSaved"+".bmp";
                File file = new File (myDir, fname);
                if (file.exists ()) file.delete (); 
                try {
                       FileOutputStream out = new FileOutputStream(file);
                       bmpgray.compress(Bitmap.CompressFormat.PNG, 100, out);
                       out.flush();
                       out.close();

                } catch (Exception e) {
                       e.printStackTrace();
                }
				*/
                
                //Edge Detection
                int w = bmp.getWidth(), h = bmp.getHeight();  
                int[] pix = new int[w * h];  
                bmp.getPixels(pix, 0, w, 0, 0, w, h);  
                int[] resultInt = LibImgFun.ImgFun(pix, w, h);  
                Bitmap resultImg = Bitmap.createBitmap(w, h, Config.RGB_565);  
                resultImg.setPixels(resultInt, 0, w, 0, 0, w, h);
                
                long performance = System.currentTimeMillis() - current;  
                imageview.setImageBitmap(bmp); 
                //imageview2.setVisibility(View.VISIBLE);
                imageview3.setVisibility(View.VISIBLE);
                //imageview2.setImageBitmap(bmpgray);
                imageview3.setImageBitmap(resultImg);
                
                ViewImage.this.setTitle("w:" + String.valueOf(bmp.getWidth())  
                        + ",h:" + String.valueOf(bmp.getHeight()) + " Time:" 
                      + String.valueOf(performance) + " ms");  
	        } 
			else if (v == btnRestore) {
	                imageview.setImageBitmap(bmp);  
	                //imageview2.setVisibility(View.GONE);
	                imageview3.setVisibility(View.GONE);
	                ViewImage.this.setTitle("Image Processing");  		 
	        }  
	    }  
	}  
		
	public static Bitmap toGrayscale(Bitmap bmpOriginal)
	{        
	    int width, height, pixel;
	    int A, R, G, B;
	    height = bmpOriginal.getHeight();
	    width = bmpOriginal.getWidth();    
	    Bitmap bmpGrayscale = Bitmap.createBitmap(width, height, Bitmap.Config.ALPHA_8);
	    
	    for(int x = 0; x < width; ++x) {
            for(int y = 0; y < height; ++y) {
                // get one pixel color
                pixel = bmpOriginal.getPixel(x, y);
                // retrieve color of all channels
                A = Color.alpha(pixel);
                R = Color.red(pixel);
                G = Color.green(pixel);
                B = Color.blue(pixel);
                // take conversion up to one single value
                R = G = B = (int)(0.299 * R + 0.587 * G + 0.114 * B);
                // set new pixel color to output bitmap
                bmpGrayscale.setPixel(x, y, Color.argb(A, R, G, B));
            }
        }
	    return bmpGrayscale;
	}


}