package average_sort;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.io.NullWritable;

public class SortPartitioner extends Partitioner<SortPair, NullWritable> {
	
	private double maxValue = 11.05;
	private double minValue = 9.0;

	@Override
	public int getPartition(SortPair key, NullWritable value, int numReduceTasks) {
		// customize which <K ,V> will go to which reducer
		// Based on defined min/max value and numReduceTasks
		// double num = key.getAverage()-9.0;
		// double a = (11.05-9.0)/numReduceTasks;
		// int task=0;
		// while(num>0){
		// 	num -= a;
		// 	++task;
		// }
		// return task;


		int num = numReduceTasks;
		double temp = key.getAverage();		
		double part = (double) (2.05 /(double) numReduceTasks);
		// int task = (int)( (temp-9.0)/part);
		num= numReduceTasks -(int)( (temp-9.0)/part);
		return num-1;
	}
}
