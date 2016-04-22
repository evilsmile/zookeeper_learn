import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooDefs;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.data.Stat;

import java.util.List;

public class Barrier implements Watcher {
	private static final String addr = "192.168.21.241:2181,192.168.21.241:2182,192.168.21.241:2183";
	private ZooKeeper zk = null;
	private Integer mutex;
	private int size = 0;
	private String root;

	public Barrier(String root, int size) {
		this.root = root;
		this.size = size;

		try {
			zk = new ZooKeeper(addr, 10 * 1000, this);
			mutex = new Integer(-1);
			Stat s = zk.exists(root, false);
			if (s == null) {
				zk.create(root, new byte[0], ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public synchronized void process(WatchedEvent event) {
		synchronized(mutex) {
			mutex.notify();
		}
	}

	public boolean enter(String name) throws Exception {
		zk.create(root + "/" + name, new byte[0], ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.EPHEMERAL);
		while (true) {
			synchronized (mutex) {
				List<String> list = zk.getChildren(root, true);
				if (list.size() < size) {
					mutex.wait();
				} else {
					return true;
				}
			}
		}
	}

	public boolean leave(String name) throws Exception {
		zk.delete(root + "/" + name, 0);
		while(true) {
			synchronized (mutex) {
				List<String> list = zk.getChildren(root, true);
				if (list.size() > 0) {
					mutex.wait();
				} else {
					return true;
				}
			}
		}
	}
}
