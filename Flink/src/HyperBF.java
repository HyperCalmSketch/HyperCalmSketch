/**
 * Hyper bloom filter, in phase 1
 * use a 2-bit timestamp to decide whether input is the first packet in the batch
 */

package hypercalm;

class HyperBFParams {
    public static final int batch_threshold = 100;
    public static final int[] simple_hash = new int[] { 557888432, 433898492, 667829344 };
    public static final int block_size = 16;
}

public class HyperBF implements java.io.Serializable {
    public int[][] sketch = new int[3][10000];

    private void clean_outdated_cell(int[] sketch, int block, int time_slice) {
        int outdated = (time_slice + 1) % 3;
        for (int i = 0; i < 16; ++i) {
            int t = (sketch[block] >> (i << 1)) & 3;
            if (t == outdated) {
                sketch[block] &= ~(3 << (i << 1));
            }
        }
    }

    private boolean insert_to_one_sketch(int[] sketch, int block, int bit_oft, int time_slice) {
        clean_outdated_cell(sketch, block, time_slice);
        // if zero flag, considered as the start of batch
        boolean ret = ((sketch[block] >> (bit_oft << 1)) & 3) == 0;
        sketch[block] &= ~(3 << (bit_oft << 1));
        sketch[block] |= (time_slice << (bit_oft << 1));

        return ret;
    }

    public boolean insert_to_sketches(int item, long timestamp) {
        boolean isFirst = false;
        int offset = HyperBFParams.batch_threshold / 3;
        for (int i = 0; i < 3; ++i) {
            // int time_slice = (int)(timestamp + offset * i / HyperBFParams.batch_threshold) % 3 + 1;
	    int time_slice = Integer.remainderUnsigned((int)(timestamp + offset * i / HyperBFParams.batch_threshold), 3) + 1;
            // int hash_v = HyperBFParams.simple_hash[i] * item % (10000 * HyperBFParams.block_size);
	    int hash_v = Integer.remainderUnsigned(HyperBFParams.simple_hash[i] * item, 10000 * HyperBFParams.block_size);
            int block = Integer.divideUnsigned(hash_v, HyperBFParams.block_size);
            int bit_oft = Integer.remainderUnsigned(hash_v, HyperBFParams.block_size);
            isFirst = isFirst || insert_to_one_sketch(sketch[i], block, bit_oft, time_slice);
        }

        return isFirst;
    }
}
