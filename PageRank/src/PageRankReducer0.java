package PageRank;

import java.io.IOException;

import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

public class PageRankReducer0 extends Reducer<Text, Text, Text, NullWritable> {
	public static int num = 0;
	public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
		context.write(key, NullWritable.get());
		++num;
	}
// 	protected void cleanup(Context context) throws IOException, InterruptedException {
// 		context.getCounter(PageRank.C.count).increment(num);
//    }
}
