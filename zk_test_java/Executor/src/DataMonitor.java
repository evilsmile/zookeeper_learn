import java.util.Arrays;
import java.io.IOException;

import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.AsyncCallback.StatCallback;
import org.apache.zookeeper.KeeperException.Code;
import org.apache.zookeeper.data.Stat;

public class DataMonitor implements Watcher, StatCallback {
	ZooKeeper zk;
	String znode;
	Watcher chainedWatcher;
	boolean dead;
	DataMonitorListener listener;

	byte prevData[];

	public DataMonitor(ZooKeeper zk, String znode, Watcher chainedWatcher, DataMonitorListener listener) {
		this.zk = zk;
		this.znode = znode;
		this.chainedWatcher = chainedWatcher;
		zk.exists(znode, true, this, null);
	}

	public interface DataMonitorListener {
		void exists(byte data[]);
		void closing(int rc);
	}

	public void process (WatchedEvent event) {
		String path = event.getPath();
		if (event.getType() == Event.EventType.None) {
			switch(event.getState()) {
				case SyncConnected:
					break;
				case Expired:
					dead = true;
					listener.closing(KeeperException.Code.SessionExpired);
					break;
			}
		} else {
			if (path != null && path.equals(znode)) {
				zk.exists(znode, true, this, null);
			}
		}
		if (chainedWatcher != null) {
			chainedWatcher.process(event);
		}
	}

	public void processResult(int rc, String path, Object ctx, Stat stat) {
		boolean exists = false;
		switch (rc) {
			case Code.Ok:
				System.out.println("Code.ok");
				exists = true;
				break;
			case Code.NoNode:
				System.out.println("Code.NoNode");
				exists = true;
				break;
			case Code.SessionExpired:
			case Code.NoAuth:
				dead = true;
				listener.closing(rc);
				return;
			default:
				zk.exists(znode, true, this, null);
				return;
		}

		byte b[] = null;
		if (exists) {
			try {
				b = zk.getData(znode, false, null);
			} catch (KeeperException e) {
				e.printStackTrace();
			} catch (InterruptedException e) {
				return;
			}
		}
		if ((b == null && b != prevData) 
				|| (b != null && !Arrays.equals(prevData, b))) {
			listener.exists(b);
			prevData = b;
		}
	}
}
