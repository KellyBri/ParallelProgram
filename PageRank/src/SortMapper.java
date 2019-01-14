package PageRank;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.io.NullWritable;

public class SortMapper extends Mapper<Text, Text, SortPair, NullWritable> {
	
	// private SortPair sp;
	
	public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
		String[] temp = value.toString().split("\\|");
		double v = Double.parseDouble( temp[1] );
		SortPair sp = new SortPair(key, v);
		context.write(sp, NullWritable.get());
	}
	
}
