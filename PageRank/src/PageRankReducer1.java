package PageRank;
import java.io.IOException;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.conf.Configuration;



public class PageRankReducer1 extends Reducer<Text, Text, Text, Text>{
	
	private static long titleNum = 0;
	
	protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
		this.titleNum = conf.getInt("titleNum", 0);
    }
	
	public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
        
        // context: key = title, value = old_pagerank|new_pagerank|link1|link2|...
        String content = "0|" + Double.toString( 1.0 / this.titleNum ) + "|";
        
		for (Text v : values){
            String t = v.toString();
			if( t.compareTo("") != 0 ) content += t + "|";
        }
		context.write(key, new Text(content) );
	}
}