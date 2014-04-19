package com.example.haveimgfun;

public class LibImgFun {    
static {     
        System.loadLibrary("ImgFun");     
       }     
      /**  
            * @param width the current view width  
            * @param height the current view height  
*/   
    public static native int[] ImgFun(int[] buf, int w, int h);    
}  