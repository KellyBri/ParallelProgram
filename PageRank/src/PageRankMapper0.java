package PageRank;
import java.io.IOException;
import java.util.StringTokenizer;

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


import java.net.URI; 
import java.io.*;



public class PageRankMapper0 extends Mapper<LongWritable, Text, Text, Text> {

	private static long num = 0;
	
	public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
        
         	
		/*  Match title pattern */  
		// No need capitalizeFirstLetter
		Pattern titlePattern = Pattern.compile("<title>(.+?)</title>");
		Matcher titleMatcher = titlePattern.matcher( unescapeXML(value.toString()) );	//get title

		if( titleMatcher.find() ){
			String title = titleMatcher.group();
			title = title.replaceAll("<title>", "").replaceAll("</title>", "");
			context.write(new Text(title), new Text(title));
			++num;
		}
	}
	
	private String unescapeXML(String input) {
		return input.replaceAll("&lt;", "<").replaceAll("&gt;", ">").replaceAll("&amp;", "&").replaceAll("&quot;", "\"").replaceAll("&apos;", "\'");
    }


	protected void cleanup(Context context) throws IOException, InterruptedException {
		//AllReduce num to PageRank.COUNTER.titleNum
 		context.getCounter(PageRank.COUNTER.titleNum).increment(num);
	}
}
