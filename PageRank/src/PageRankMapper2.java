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
import java.lang.Math;


public class PageRankMapper2 extends Mapper<LongWritable, Text, Text, Text> {

    private static long err = 0;
    
	
	public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
        String[] val = value.toString().split("\t");
        String title = val[0];
        String PR_links = val[1];

        String[] temp = PR_links.split("\\|");
        double oldPR = Double.parseDouble( temp[0] );
        double newPR = Double.parseDouble( temp[1] );

        err += Math.abs(newPR - oldPR) * Math.pow(10, 18);

        double PR = newPR / (temp.length - 2);
        String links = "|" + Double.toString(newPR);

        for(int i=2; i<temp.length; ++i){
            context.write( new Text(temp[i]), new Text( Double.toString(PR) ) );
            links += "|" + temp[i];
        }
        context.write( new Text(title), new Text(links) );


        // String[] key_value = value.toString().split("\t");
		// String k = key_value[0];
		// String v = key_value[1];
		
		// String[] value_parts = v.split("\\|");
		// double oldPR = Double.parseDouble(value_parts[0]);
		// double newPR = Double.parseDouble(value_parts[1]);
		
		// if(newPR > oldPR) err += (long)((newPR - oldPR) * Math.pow(10, 18));
		// else err += (long)((oldPR - newPR) * Math.pow(10, 18));
		
		// String links = "|" + value_parts[1] + "|";
		// if(value_parts.length > 2){
		// 	int degree = value_parts.length - 2;
		// 	for(int i=2; i < value_parts.length; i++){
		// 		String pr = String.valueOf(newPR / (double)degree);
		// 		context.write(new Text(value_parts[i]), new Text(pr));
		// 		links += value_parts[i];
		// 		links += "|";
		// 	}
		// }
		// context.write(new Text(k), new Text(links));
	}
    
    

	protected void cleanup(Context context) throws IOException, InterruptedException {
 		context.getCounter(PageRank.COUNTER.err).increment(err);
	}
}
