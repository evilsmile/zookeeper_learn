import java.util.List;
import java.util.Random;
import java.nio.ByteBuffer;
import java.io.IOException;

import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooDefs;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.data.Stat;

class SyncPrimitive implements Watcher {
	static ZooKeeper zk = null;
	static Integer mutex;
	//static Integer mutex;

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

public class Barrier extends SyncPrimitive {
	private int size = 0;

	private String name;

	public Barrier(String addr, String root, int size) {
		super(addr);
		this.root = root;
		this.size = size;

		if (zk != null) {
			try {
				Stat s = zk.exists(root, false);
				if (s == null) {
					zk.create(root, new byte[0], ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT);
				}
			} catch (KeeperException e) {
				System.out.println("Keeper exception when instantiating queue: " + e.toString());
			} catch (InterruptedException e){
				System.out.println("Interrupted exception");
				e.printStackTrace();
			} 
		}
	}

	public boolean enter(String name) throws KeeperException, InterruptedException {
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

	public boolean leave(String name) throws KeeperException, InterruptedException{
		zk.delete(root + "/" + name, 0);
		while(true) {
			synchronized (mutex) {
				List<String> list = zk.getChildren(root, true);
				System.out.println(name + " get lock. now size: " + list.size());
				if (list.size() > 0) {
					mutex.wait();
				} else {
					mutex.notify();
					return true;
				}
			}
		}
	}
}
