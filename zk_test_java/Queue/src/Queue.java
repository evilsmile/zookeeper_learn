import java.util.List;
import java.nio.ByteBuffer;
import java.io.IOException;

import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooDefs;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.data.Stat;
import org.apache.zookeeper.KeeperException;

class SyncPrimitive implements Watcher {
	static ZooKeeper zk = null;
	static Integer mutex;

	protected String root;

	SyncPrimitive(String addr) {
		if (zk == null) {
			System.out.println("Starting ZK:");
			try {
				zk = new ZooKeeper(addr, 3000, this);
				mutex = new Integer(-1);
				System.out.println("Finished starting ZK: " + zk);
			} catch (IOException e) {
				System.out.println(e.toString());
				zk = null;
			}
		}
	}
	public synchronized void process(WatchedEvent event) {
		System.out.println(event.toString());
		synchronized(mutex) {
			mutex.notify();
		}
	}
}

public class Queue extends SyncPrimitive {

	Queue(String addr, String name) {
		super(addr);
		this.root = name;
		if (zk != null) {
			try { 
				Stat s = zk.exists(root, false);
				if (s == null) {
					zk.create(root, new byte[0], ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT);
				}
			} catch (KeeperException e) {
				System.out.println("Keeper exception when instantiating queue: " + e.toString());
			} catch (InterruptedException e) {
				System.out.println("Interrupted exception");
			}
		}
	}

	boolean produce(int i) throws KeeperException, InterruptedException {
		ByteBuffer b = ByteBuffer.allocate(4);
		byte[] value;

		b.putInt(i);
		value = b.array();
		zk.create(root + "/element", value, ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT_SEQUENTIAL);
		return true;
	}

	int consume() throws KeeperException, InterruptedException {
		int retvalue = -1;
		Stat stat = null;

		while (true) {
			synchronized(mutex) {
				List<String> list = zk.getChildren(root, true);
				if (list.size() == 0) {
					System.out.println("Going to wait");
					mutex.wait();
				} else {
					String min = new String(list.get(0).substring(7));
					for (String s : list) {
						String tmpValue = new String(s.substring(7));
						if (tmpValue.compareTo(min) < 0) min = tmpValue;
					}
					System.out.println("Temporary value: " + root + "/element" + min);
					byte[] b = zk.getData(root + "/element" + min, false, stat);
					zk.delete(root + "/element" + min, 0);
					ByteBuffer buffer = ByteBuffer.wrap(b);
					retvalue = buffer.getInt();

					return retvalue;
				}
			}
		}
	}
}
