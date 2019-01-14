package PageRank;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.io.NullWritable;

public class PageRankPartitioner0 extends Partitioner<SortPair, NullWritable> {
	@Override
	public int getPartition(SortPair key, NullWritable value, int numReduceTasks) {
		// customize which <K ,V> will go to which reducer
		double k = key.getAverage();
		if(k>=0 && k<0.5) return 0;
		if(k>=0.5 && k<1) return 1;
		if(k>=1 && k<1.5) return 2;
		if(k>=1.5 && k<2) return 3;
		if(k>=2 && k<2.5) return 4;
		if(k>=2.5 && k<3) return 5;
		if(k>=3 && k<3.5) return 6;
		if(k>=3.5 && k<4) return 7;
		if(k>=4 && k<4.5) return 8;
		if(k>=4.5 && k<5) return 9;
		if(k>=5 && k<5.5) return 10;
		if(k>=5.5 && k<6) return 11;
		if(k>=6 && k<6.5) return 12;
		if(k>=6.5 && k<7) return 13;
		if(k>=7 && k<7.5) return 14;
		if(k>=7.5 && k<8) return 15;
		if(k>=8 && k<8.5) return 16;

		if(k>=8.5 && k<9) return 17;
		if(k>=9 && k<9.5) return 18;
		if(k>=9.5 && k<10) return 19;
		if(k>=10 && k<10.5) return 20;
		if(k>=10.5 && k<11) return 21;
		if(k>=11 && k<11.5) return 22;
		if(k>=11.5 && k<12) return 23;
		if(k>=12 && k<12.5) return 24;
		if(k>=12.5 && k<13) return 25;
		if(k>=13 && k<13.5) return 26;
		if(k>=13.5 && k<14) return 27;
		if(k>=14 && k<14.5) return 28;
		if(k>=14.5 && k<15) return 29;
		if(k>=15 && k<15.5) return 30;
		else return 31;

	}
}
