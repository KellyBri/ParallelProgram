package PageRank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;

public class JOB1{

    int titleNum;
    String inputPath, outputPath;

    public JOB1(String in, String out, int num){
        this.inputPath  = in;
        this.outputPath = out;
        this.titleNum = num;
    }
    public int run() throws Exception {
        Configuration conf = new Configuration();
        conf.setInt("titleNum", this.titleNum);
		Job job = Job.getInstance(conf, "JOB1");
		job.setJarByClass(JOB1.class);
		
		// set the class of each stage in mapreduce
		job.setMapperClass(PageRankMapper1.class);
		job.setReducerClass(PageRankReducer1.class);
		
		/* set the output class of Mapper and Reducer */
		// Mapper
		job.setInputFormatClass(TextInputFormat.class);
		job.setMapOutputKeyClass(Text.class);
		job.setMapOutputValueClass(Text.class);
		// Reducer
		job.setOutputKeyClass(Text.class);
		job.setOutputValueClass(Text.class);
		
		// set the number of reducer
		job.setNumReduceTasks(32);
		
		// add input/output path
		FileInputFormat.addInputPath  (job, new Path( this.inputPath ) );
		FileOutputFormat.setOutputPath(job, new Path( this.outputPath ));
		
        job.waitForCompletion(true);
        // return 0;
		return (int)job.getCounters().findCounter(PageRank.COUNTER.dangleNum).getValue();
    }
	
}
