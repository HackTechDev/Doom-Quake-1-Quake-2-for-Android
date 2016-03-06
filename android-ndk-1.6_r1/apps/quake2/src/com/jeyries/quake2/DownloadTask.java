package com.jeyries.quake2;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PushbackInputStream;
import java.io.RandomAccessFile;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.os.SystemClock; 
import android.util.Log;


class DownloadTask extends AsyncTask<Void, String, Void> {
	
	private Context context;
	private ProgressDialog pd;
	
	public boolean please_abort = false;
	
	private String url = "ftp://ftp.idsoftware.com/idstuff/quake2/q2-314-demo-x86.exe";
	private String demofile = "/sdcard/baseq2/q2-314-demo-x86.exe";
	private String pakfile = "/sdcard/baseq2/pak0.pak";
	
	
	
	public DownloadTask set_context(Context context, String url){
		this.context = context;
		this.url = url;
		return this;
	}
	
	@Override
	protected void  onPreExecute  (){
     
	     pd = new ProgressDialog(context);
	     
	     pd.setTitle("Downloading data file ...");
	     pd.setMessage("starting");
	     pd.setIndeterminate(true);
	     pd.setCancelable(true);
	     
	     pd.setOnDismissListener( new DialogInterface.OnDismissListener(){
			//@Override
			public void onDismiss(DialogInterface dialog) {
				Log.i( "DownloadTask.java", "onDismiss");
				please_abort = true;		
			}    	 
	     });
	     
	     pd.setOnCancelListener( new DialogInterface.OnCancelListener(){
				//@Override
				public void onCancel(DialogInterface dialog) {
					Log.i( "DownloadTask.java", "onCancel");
					please_abort = true;		
				}    	 
		 });
	     
	     
			pd.setButton(ProgressDialog.BUTTON_POSITIVE, "Cancel", new DialogInterface.OnClickListener() {
	           public void onClick(DialogInterface dialog, int id) {
	        	   pd.dismiss();
	        	   please_abort = true;
	           }
	       });
			
			pd.show();
	 
	}
	
	
	public static String printSize( int size ){
		
		if ( size >= (1<<20) )
			return String.format("%.1f MB", size * (1.0/(1<<20)));
		
		if ( size >= (1<<10) )
			return String.format("%.1f KB", size * (1.0/(1<<10)));
		
		return String.format("%d bytes", size);
		
	}
	


	
	private void download_demo() throws Exception{
		
		Log.i( "DownloadTask.java", "starting to download "+ url);
		
	    if (new File(demofile).exists()){
	    	Log.i( "DownloadTask.java", demofile + " already there. skipping.");
	    	return;
	    }
	        		
	        		
		/// setup output directory		
		new File("/sdcard/baseq2").mkdirs();
		
       	InputStream     is = null;
    	FileOutputStream        fos = null;
    		    		
		is = new URL(url).openStream();


    	fos = new FileOutputStream ( demofile+".part");

    	byte[]  buffer = new byte [4096];
    	
    	int totalcount =0;
    	
    	long tprint = SystemClock.uptimeMillis();
    	int partialcount = 0;
    	
    	while(true){
    		
    		 if (please_abort)
    			 throw new Exception("aborting") ;	    	    	
	    	 
    		
    		int count = is.read (buffer);
    		//Log.i( "DownloadTask.java", "received " + count + " bytes");
    		
    	    if ( count<=0 ) break;
    	    fos.write (buffer, 0, count);
    	    
    	    totalcount += count;
    	    partialcount += count;
    	    
    	    long tnow =  SystemClock.uptimeMillis();
    	    if ( (tnow-tprint)> 1000) {
    	    	
    	    	float size_MB = totalcount * (1.0f/(1<<20));
    	    	float speed_KB = partialcount  * (1.0f/(1<<10)) * ((tnow-tprint)/1000.0f);
    	    	
    	    	publishProgress( String.format("downloaded %.1f MB (%.1f KB/sec)",
    	    			size_MB, speed_KB));

    	    	tprint = tnow;
    	    	partialcount = 0;
    	    	
    	    }
    	       	    
    	   
    	}
    	
    	is.close();
    	fos.close();
    	

    	
    	/// rename part file
    	
    	new File(demofile+".part" )
    		.renameTo(new File(demofile));
    	
    	// done
    	publishProgress("download done" );
    	
		SystemClock.sleep(2000);
    	
	}
	
	private void extract_data() throws Exception{
		

		Log.i( "DownloadTask.java", "extracting PAK data");

		
		/// setup output directory		
		new File("/sdcard/baseq2").mkdirs();
		
		
		ZipFile file = new ZipFile  (demofile);
    	    	
    	extract_directory( file, "Install/Data/baseq2/players", "/sdcard/baseq2/players");

       	extract_file( file, "Install/Data/baseq2/pak0.pak", pakfile);
  	
    	file.close();
    	
    	// done
    	publishProgress("extract done" );

		SystemClock.sleep(2000);
		
	}
	
	private void extract_directory( ZipFile file, String entry_name, String output_name ) throws Exception{
		
		
		Log.i( "DownloadTask.java", "extracting " + entry_name + " to " + output_name);


	
		for (Enumeration e = file.entries(); e.hasMoreElements(); ) {
    	

			if (please_abort)
				throw new Exception("aborting") ;	  	    	
		 
			ZipEntry entry = (ZipEntry)(e.nextElement());
			
			String name = entry.getName();
			if (!name.startsWith(entry_name))
				continue;

			String out_name = name.replace(entry_name, output_name);
			
			extract_file( file, name, out_name );

			
		}
   		
    	
	
		
	}
	
	private void extract_file( ZipFile file, String entry_name, String output_name ) throws Exception{
		
		
		
		
		Log.i( "DownloadTask.java", "extracting " + entry_name + " to " + output_name);

		String short_name = new File(output_name).getName();
		
		// never overwrite
		/*
	    if (new File(output_name).exists()){
	    	Log.i( "DownloadTask.java", output_name + " already there. skipping.");
	    	return;
	    }
	    */

		
	    // create output directory
		new File(output_name).getParentFile().mkdirs();
		
		ZipEntry entry = file.getEntry(entry_name);
		
		if ( entry.isDirectory() ){	
			Log.i( "DownloadTask.java", entry_name + " is a directory");
			new File(output_name).mkdir();
			return;
		}
		
				
       	InputStream is = null;
    	FileOutputStream  fos = null;
    		    		
		is = file.getInputStream(entry);
		
    	fos = new FileOutputStream ( output_name+".part" );

    	byte[]  buffer = new byte [4096];
    	
    	int totalcount =0;
    	
    	long tprint = SystemClock.uptimeMillis();
    	
    	while(true){
    		
    		 if (please_abort)
    			 throw new Exception("aborting") ;	  	    	
	    	 
    		
    		int count = is.read (buffer);
    		//Log.i( "DownloadTask.java", "extracted " + count + " bytes");
    		
    	    if ( count<=0 ) break;
    	    fos.write (buffer, 0, count);
    	    
    	    totalcount += count;
    	    
    	    long tnow =  SystemClock.uptimeMillis();
    	    if ( (tnow-tprint)> 1000) {
    	    	
    	    	float size_MB = totalcount * (1.0f/(1<<20));
    	    	
    	    	publishProgress( String.format("%s : extracted %.1f MB",
    	    			short_name, size_MB));

    	    	tprint = tnow;
    	    	
    	    }
    	    
    	   
    	}
    	
    	
    	is.close();
    	fos.close();
    	    	
    	/// rename part file
    	
    	new File(output_name+".part" )
    		.renameTo(new File(output_name));
    	
    	// done
    	publishProgress( String.format("%s : done.",
    			short_name));
	
		
	}
	
	private void download_zipstream() throws Exception{
		
		Log.i( "DownloadTask.java", "download zip stream from "+ url);
		
	    if (new File(pakfile).exists()){
	    	Log.i( "DownloadTask.java", pakfile + " already there. skipping.");
	    	return;
	    }
		
		/// setup output directory		
		new File("/sdcard/baseq2").mkdirs();
	
		/*InputStream is = 
				new BufferedInputStream( new FileInputStream(demofile)) ;
				//new FileInputStream("/sdcard/baseq2/infinity.zip") ;
				//new URL(url).openStream () ;
		*/
				
		PushbackInputStream is = new PushbackInputStream(
				new BufferedInputStream( new FileInputStream(demofile)),
				10 );
		
		////////////// skip until first header
		// P K \0x03 \0x04
		// need PushbackInputStream unread
		// or BufferedInputStream mark/reset
		
		
		Log.i( "DownloadTask.java", "search for zip header");
		/*
		if ( !is.markSupported() ){
			throw new Exception("mark not supported") ;
		}*/
		
		//byte header[] = { 0x50, 0x4B, 0x03, 0x04 };
		//byte header[] = { 0x50, 0x4B, 0x01, 0x02 };
		byte header[] = { 0x50, 0x4B, 0x05, 0x06 };
		
		int pos_header = 0;
		int pos_archive = 0;
		while(true) {
			
			if (please_abort)
	   			 throw new Exception("aborting") ;	  	    	
	    	 
			
			/*if (pos_header==0)
				is.mark(10);
			*/
			
			int c = is.read();		
			
			if (c==header[pos_header]){			
				
				//Log.i( "DownloadTask.java", String.format("found 0x%02x at 0x%08x",c, pos_archive));
				pos_archive ++;
				pos_header ++;
				if (pos_header>=header.length){
					break;
				}
				
			} else {
				
				pos_archive ++;
				pos_header = 0;
			}
			
			
		}
		
		//is.reset();
		is.unread(header); pos_archive -= header.length;
		
		Log.i( "DownloadTask.java", String.format("zip header found after 0x%08x bytes", pos_archive));
		// offset 0xa83
		
		/*
		for (int i=0;i<10;i++){
			int c = is.read();		
			Log.i( "DownloadTask.java", String.format("found 0x%02x at 0x%08x",c, pos_archive));
			pos_archive ++;
		}*/
		
		/////////////
		
		
		ZipInputStream zipstream = new ZipInputStream( is );
		
		
		ZipEntry  entry;
		
		while(true) {
			
			if (please_abort)
	   			 throw new Exception("aborting") ;	  	    	
	    	 			
			entry = zipstream.getNextEntry();
			if (entry==null){
				throw new Exception("pak0.pak not found in archive");
			}
					
			Log.i( "DownloadTask.java", "found "+ entry.getName());
		
			if ("Install/Data/baseq2/pak0.pak".equals(entry.getName())){
				break;
			}
		}
		
		
				
    	FileOutputStream  fos = null;		    		
		
		/// setup output directory		
		new File("/sdcard/baseq2").mkdirs();

    	fos = new FileOutputStream ( pakfile+".part" );

    	byte[]  buffer = new byte [4096];
    	
    	
    	int totalcount =0;
    	
    	while(true){
    		
    		 if (please_abort)
    			 throw new Exception("aborting") ;	  	    	
	    	 
    		
    		int count = zipstream.read (buffer);
    		//Log.i( "DownloadTask.java", "extracted " + count + " bytes");
    		
    	    if ( count<=0 ) break;
    	    fos.write (buffer, 0, count);
    	    
    	    totalcount += count;
    	    publishProgress("extracted " + printSize(totalcount) );
    	    
    	   
    	}
    	
    	
    	zipstream.close();
    	is.close();
    	fos.close();


    	
    	/// rename part file
    	
    	new File(pakfile+".part" )
    		.renameTo(new File(pakfile));
    	
    	// done
    	publishProgress("extract done" );
    	
		SystemClock.sleep(2000);
		
	}
	
	private void unpak() throws Exception{
	    	
		Log.i( "DownloadTask.java", "unpak "+ pakfile);
	    
		String colormap = "/sdcard/baseq2/pics/colormap.pcx";
	    if (new File(colormap).exists()){
	    	Log.i( "DownloadTask.java", colormap + " already there. skipping.");
	    	return;
	    }
	    	
	    	FS.pack_t pack = FS.LoadPackFile( pakfile);
	    	
	    	RandomAccessFile handle = new RandomAccessFile( pakfile,"r" );
	    	
	    	byte[] buffer = new byte[4096];
	    	
	    	//for ( Object obj : pack.files.entrySet() ){
	    	//	packfile_t entry = (packfile_t)obj;
	    	for (Enumeration e = pack.files.elements(); e.hasMoreElements(); ) {
	    		
	    		 if (please_abort)
	    			 throw new Exception("aborting") ;	  	    	
		    	 
	    		 
	    		FS.packfile_t entry = (FS.packfile_t)(e.nextElement());
	    		
	    		//System.out.println(entry);
	    		
	    		 /*byte[] data = FS.LoadFile(entry.name);
	    		 if ( data==null) continue;
	    		 */
	    		handle.seek(entry.filepos);
	    		    		 
	    		File file = new File("/sdcard/baseq2/"+entry.name);
	    		file = file.getCanonicalFile();
	    		
	    		File parent = file.getParentFile();
	    		
	    		if (parent!=null)
	    			parent.mkdirs();
	    		
	    		// mkdir -p
	    		/*File parent = file.getParentFile();
	    		
	    		while ( true ){
	    			if (!parent.exists()){
	    				parent.m
	    			}
	    		}*/
	    		
				
				
				// copy data

				
				publishProgress("unpaking " + entry.name );
	    	    
				FileOutputStream fos = new FileOutputStream(file);		

				int totalcount = 0;
				
				while( totalcount< entry.filelen){
					int count = handle.read (buffer,0, Math.min(buffer.length,entry.filelen-totalcount) );
		    		
		    	    if ( count<=0 ) break;
		    	    fos.write (buffer, 0, count);
		    	    
		    	    totalcount += count;
				}
				
				if (totalcount!=entry.filelen)
					throw new Exception("wrong totalcount") ;	  
				
				fos.close();
				
					
				
	    	}
	    	
	    	handle.close();
	    	
	    	/// rename pak file
	    	
	    	new File(pakfile )
	    		.renameTo(new File(pakfile+".backup"));
	    	
	    	// done
	    	publishProgress("unpak done" );
	    	
			SystemClock.sleep(2000);
	    }
	   
	
	@Override
	protected Void doInBackground(Void... unused) {
		
	
    	// q2-314-demo-x86.exe : 39015499 bytes
    	// pak0.pak : 49951322 bytes
    	


    	try { 	
    		
    		long t = SystemClock.uptimeMillis();

    		download_demo();
    		
    		extract_data();   		
    		
    		//download_zipstream();
    		
    		//unpak();
    		
    		t = SystemClock.uptimeMillis() - t;
    		
    		Log.i( "DownloadTask.java", "done in " + t + " ms");
	    	
    	} catch (Exception e) {

			e.printStackTrace();
			
			publishProgress("Error: " + e );
		}
    	
		return(null);
	}
	
	@Override
	protected void onProgressUpdate(String... progress) {
		Log.i( "DownloadTask.java", progress[0]);
		pd.setMessage( progress[0]);
	}
	
	@Override
	protected void onPostExecute(Void unused) {
		//Log.i( "DownloadTask.java", "onPostExecute " );
		/*
		Toast
			.makeText(Quake2.this, "Done!", Toast.LENGTH_SHORT)
			.show();
			
		*/
		 // showDialog("Download Done !");
		//dismissDialog(DIALOG_DOWNLOAD);
		
		 //pd.dismiss();
		
		pd.getButton(ProgressDialog.BUTTON_POSITIVE).setText("Done");
		/*pd.setButton(ProgressDialog.BUTTON_POSITIVE, "Done", new DialogInterface.OnClickListener() {
	           public void onClick(DialogInterface dialog, int id) {
	        	   pd.dismiss();
	           }
	       });*/
	       
	}
}
