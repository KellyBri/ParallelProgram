package PageRank;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.WritableComparator;

public class PageRankKeyComparator extends WritableComparator {
	
	public PageRankKeyComparator() {
		super(Text.class, true);
	}	
	
	public int compare(WritableComparable o1, WritableComparable o2) {
		Text key1 = (Text) o1;
		Text key2 = (Text) o2;

		char first = key1.toString().charAt(0);
		char two   = key2.toString().charAt(0);
		// TODO Order by A -> a -> B -> b ....
		if(first == two) return 0;

		char a = Character.toLowerCase(first);
		char b = Character.toLowerCase(two);
		if( a == b ){
			if( first < two ) return -1;
			else return 1;
		}else if( a < b ) return -1;
		// if( first < two ) return -1;
		return 1;
	}
}
