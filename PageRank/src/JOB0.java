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

public class JOB0{

    String inputPath, outputPath;

    public JOB0(String in, String out){
        this.inputPath = in;
        this.outputPath = out;
    }
    public int run() throws Exception {
		Configuration conf = new Configuration();
		Job job = Job.getInstance(conf, "JOB0");
		job.setJarByClass(JOB0.class);
		
		// set the class of each stage in mapreduce
		job.setMapperClass(PageRankMapper0.class);
		// job.setPartitionerClass(PageRankPartitioner0.class);
		job.setReducerClass(PageRankReducer0.class);
		
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
		return (int)job.getCounters().findCounter(PageRank.COUNTER.titleNum).getValue();
		// return (int)job.getCounters().findCounter(PageRank.C.count).getValue();
    }
	
}
