package iot.pot.utils;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public interface ExecutorsPool {
    ExecutorService executorService = Executors.newCachedThreadPool();
}
