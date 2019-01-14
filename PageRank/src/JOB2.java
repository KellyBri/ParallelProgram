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

import java.lang.Math;

public class JOB2{

    int titleNum;
    // double sum2;
    String inputPath, outputPath;

    public JOB2(String in, String out, int num){
        this.inputPath  = in;
        this.outputPath = out;
        this.titleNum = num;
        // this.sum2 = sum2;
    }
    public long run() throws Exception {
        Configuration conf = new Configuration();
        conf.setInt("titleNum", this.titleNum);
        conf.setDouble("sum2", PageRank.sum2);
		Job job = Job.getInstance(conf, "JOB2");
		job.setJarByClass(JOB2.class);
		
		// set the class of each stage in map reduce
		job.setMapperClass (PageRankMapper2.class);
		job.setReducerClass(PageRankReducer2.class);
		
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
        PageRank.err_s = (double)job.getCounters().findCounter(PageRank.COUNTER.err ).getValue() / Math.pow(10, 18);
        PageRank.sum2  = (double)job.getCounters().findCounter(PageRank.COUNTER.sum2).getValue() / Math.pow(10, 18);
        return 0;
    }
}
