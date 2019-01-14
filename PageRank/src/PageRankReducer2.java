package PageRank;
import java.io.IOException;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.conf.Configuration;



public class PageRankReducer2 extends Reducer<Text, Text, Text, Text>{
	
	private static long titleNum = 0;
	private static double sum2 = 0;
    private static double newSum2 = 0;

	protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
		this.titleNum = conf.getInt("titleNum", 0);
        this.sum2 = conf.getDouble("sum2", 0);
    }
    
	
	public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
        
        String oldPR = "";
        String links = "";
        double sum1  = 0;
        for(Text value: values){
            String v = value.toString();
            
            if( v.charAt(0) == '|' ){
                String[] temp = v.split("\\|");
                oldPR += temp[1] + "|";
                for(int i=2; i<temp.length; ++i){
                    links += temp[i] + "|";
                }
            }else{
                sum1 += Double.parseDouble( value.toString() );
            }
        }

        double newPR = 0.15 / this.titleNum + 0.85 * ( sum1 + sum2 / this.titleNum );
        if( links.compareTo("") == 0 ) newSum2 += newPR;

        String newLinks = oldPR + Double.toString( newPR ) + "|" + links;
        
        context.write( key, new Text(newLinks) );
	}

    protected void cleanup(Context context) throws IOException, InterruptedException {
        // long s = (long)( newSum2 * Math.pow(10, 18) );
        long s = (long)(newSum2 * Math.pow(10, 18));
        context.getCounter(PageRank.COUNTER.sum2).increment( s );
        
    }
}