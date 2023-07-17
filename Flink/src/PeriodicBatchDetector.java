package hypercalm;

import org.apache.flink.api.common.state.ValueState;
import org.apache.flink.api.common.state.ValueStateDescriptor;
import org.apache.flink.api.common.typeinfo.Types;
import org.apache.flink.configuration.Configuration;
import org.apache.flink.streaming.api.functions.KeyedProcessFunction;
import org.apache.flink.util.Collector;
import org.apache.flink.api.java.tuple.Tuple2;

public class PeriodicBatchDetector extends KeyedProcessFunction<Integer, Tuple2<Integer, Long>, PeriodicBatch> {
    public HyperBF hyper_bf = new HyperBF();
    public CalmSpaceSaving CalmSS = new CalmSpaceSaving();

    @Override
    public void processElement (
      Tuple2<Integer, Long> item,
      Context ctx,
      Collector<PeriodicBatch> collector) throws Exception {
        // process each element in the stream
	int id = item.f0;
        long timestamp = item.f1;
        if (hyper_bf.insert_to_sketches(id, timestamp)) {
            // be the first in the batch
            PeriodicBatch batch = CalmSS.insert(id, timestamp);
	    if (batch.interval != -1) { 
            	collector.collect(batch);
	    }
        }
    }
}
