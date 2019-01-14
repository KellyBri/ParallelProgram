package PageRank;
import java.io.IOException;

import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;

import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.conf.Configuration;


import java.io.*;
import java.util.Set;
import java.util.HashSet;
import java.lang.Object;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import java.util.*;


public class PageRankMapper1 extends Mapper<LongWritable, Text, Text, Text> {

    private Set<String> titleSet = new HashSet<String>();
    private static long num = 0;
    private static final Log LOG = LogFactory.getLog(PageRankMapper1.class);
    
    public void setup(Context context) throws IOException{
        
        //read file
        Path inputPath = new Path( "PageRank/output/title.txt" );
        Configuration cong = new Configuration();
        FileSystem fs = FileSystem.get( cong );
        BufferedReader buffer = new BufferedReader( new InputStreamReader( fs.open(inputPath), "UTF8" ) );
        
        //////////////////////////////////////
        // ArrayList <String> QQ = new ArrayList(); 
        // String line = buffer.readLine();
        // StringBuilder more = new StringBuilder();
        // int i = 0;
        // while(line != null){
        //     ++i;
		// 	String[] title = line.split("\t");
        //     QQ.add( title[0] );
		// 	line = buffer.readLine();
        // }
        // Collections.sort(QQ);
        // for(int j = 1; j < QQ.size(); ++j){
        //     if( QQ.get(j).compareTo( QQ.get(j-1) )==0 ) more.append( "\n"+QQ.get(j) + "\n" );
        // }
        //////////////////////////////////////

        //get titles from title.txt and add title into HashSet
        titleSet.clear();
        String line = buffer.readLine();
        StringBuilder more = new StringBuilder();
        int i = 0;
        while(line != null){
            ++i;
			String[] title = line.split("\t");
            // if( titleSet.contains(title[0]) ) more.append(title[0]+"\n");
            if( !titleSet.add(title[0]) ) more.append(title[0]+"\n");
			line = buffer.readLine();
        }
        // PageRank.more = Integer.toString(i) + "\n" + more.toString();
        // context.getCounter(PageRank.COUNTER.dangleNum).setValue( titleSet.size() );

    }
	
	public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
        
         	
		/*  Match title pattern */  
		// No need capitalizeFirstLetter
		Pattern titlePattern = Pattern.compile("<title>(.+?)</title>");
		Matcher titleMatcher = titlePattern.matcher( unescapeXML(value.toString()) );	//get title

        String title;
		if( titleMatcher.find() ){
			title = titleMatcher.group();
            title = title.replaceAll("<title>", "").replaceAll("</title>", "");
            /*  Match link pattern */
            Pattern linkPattern = Pattern.compile("\\[\\[(.+?)([\\|#]|\\]\\])");
            Matcher linkMatcher = linkPattern.matcher( unescapeXML(value.toString()) );	//get link

            boolean hasOutEdge = false;
            while( linkMatcher.find() ){
                String link = capitalizeFirstLetter( linkMatcher.group(1) );
                // String link = linkMatcher.group(1);
                //check if the link exists
                if( titleSet.contains(link) ){
                    // ++num;
                    context.write( new Text(title), new Text(link) );
                    hasOutEdge = true;
                }
            }

            // if( linkMatcher.find() ){
            //     for(int i=0; i<linkMatcher.groupCount(); ++i){
            //         String link = capitalizeFirstLetter( linkMatcher.group(i) );
            //         //check if the link exists
            //         if( titleSet.contains(link) ){
            //             // ++num;
            //             context.write( new Text(title), new Text(link) );
            //             hasOutEdge = true;
            //         }
            //     }
            // }


            
            if( !hasOutEdge ){
                context.write( new Text(title), new Text("") );
                ++num;
            }
		}
	}
    
    
	private String unescapeXML(String input) {
		return input.replaceAll("&lt;", "<").replaceAll("&gt;", ">").replaceAll("&amp;", "&").replaceAll("&quot;", "\"").replaceAll("&apos;", "\'");
    }

    private String capitalizeFirstLetter(String input){

    	char firstChar = input.charAt(0);
        if ( firstChar >= 'a' && firstChar <='z'){
            if ( input.length() == 1 ) return input.toUpperCase();
            else return input.substring(0, 1).toUpperCase() + input.substring(1);
        }
        else return input;
    }

	protected void cleanup(Context context) throws IOException, InterruptedException {
 		context.getCounter(PageRank.COUNTER.dangleNum).increment(num);
	}
}
