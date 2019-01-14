//
package PageRank;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import static java.lang.System.out;
import org.apache.commons.lang.math.NumberUtils;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileUtil;


import java.util.*;

public class PageRank {

	public static enum COUNTER{   
		titleNum, dangleNum, err, sum2
	}
	public enum C {
		count
	}
	// public static String more;
	public static double err_s = 0;
	public static double sum2  = 0;

	public static void main(String[] args) throws Exception {

		String inputPath  = args[0];
		String outputPath = args[1];
		// int iteration = -1;
		// if( NumberUtils.isNumber(args[2]) ) iteration = Integer.parseInt( args[2] );
		int iteration = Integer.parseInt( args[2] );


		// get all titles from input
		JOB0 job0 = new JOB0(inputPath, outputPath+"/title");
		int titleNum = job0.run();
		// Copy all files in a directory to one output file (merge), and also delete source
		Configuration conf = new Configuration();
		FileSystem fileSystem = FileSystem.get(conf);
		Path source0 = new Path(outputPath + "/title");
		Path target0 = new Path(outputPath + "/title.txt");
		FileUtil.copyMerge(fileSystem, source0, fileSystem, target0, true, conf, null);



		// get all links from links and calculate the PageRank of each vertex at iteration 0
		JOB1 job1 = new JOB1(inputPath, outputPath+"/iteration0", titleNum);
		int dangleNum = job1.run();

		

		List<Double> error = new ArrayList<Double>();

		int i=0;
		sum2 = (double)dangleNum / titleNum;
		if(iteration == -1){
			
			while(true){
				fileSystem.delete( new Path( outputPath + "/iteration" + Integer.toString(i-1) ), true);
				String input  = outputPath + "/iteration" + Integer.toString(i);
				String output = outputPath + "/iteration" + Integer.toString(i+1); 
				JOB2 job2 = new JOB2(input, output, titleNum);
				job2.run();
				error.add(err_s);
				if( err_s < 0.001 ) break;
				++i;
				
			}

		}else{

			for(i=0; i<iteration; ++i){
				fileSystem.delete( new Path( outputPath + "/iteration" + Integer.toString(i-1) ), true);
				String input  = outputPath + "/iteration" + Integer.toString(i);
				String output = outputPath + "/iteration" + Integer.toString(i+1); 
				JOB2 job2 = new JOB2(input, output, titleNum);
				job2.run();
				error.add(err_s);
			}

		}

		
		Sort sort = new Sort();
		sort.run(outputPath + "/iteration" + Integer.toString(i), outputPath+"/final");



		for(int j=0; j<error.size(); ++j){
			out.println( error.get(j) );
		}
	}
}
