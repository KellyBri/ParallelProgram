package wordcount;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

public class WordCountMapper extends Mapper<LongWritable, Text, Text, IntWritable> {
	
	private IntWritable one = new IntWritable(1);
	private Text word = new Text();
	
	public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
		
		// we simply use StringTokenizer to split the words for us.
		StringTokenizer itr = new StringTokenizer(value.toString());
		while (itr.hasMoreTokens()) {
			// TODO: find the first character of word, and create K,V pair for it.
			String toProcess = itr.nextToken();
			// we only need to handle Aa~Zz
			if( Character.isLetter( toProcess.charAt(0) ) ){
				// create <K, V> pair
				word.set( toProcess.substring(0,1) );
				context.write(word,one);
			}
		}
	}
}
