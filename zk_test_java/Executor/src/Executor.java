import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooKeeper;

public class Executor implements Watcher, Runnable, DataMonitor.DataMonitorListener {

	String znode;
	DataMonitor dm;
	ZooKeeper zk;
	String filename;
	String exec[];
	Process child;

	private static final String addr = "192.168.21.241:2181,192.168.21.241:2182,192.168.21.241:2183";

	public Executor(String znode, String filename, String exec[]) throws KeeperException, IOException {
		this.filename = filename;
		this.exec = exec;
		zk = new ZooKeeper(addr, 3000, this);
		dm = new DataMonitor(zk, znode, null, this);
	}

	public static void main(String[] args) {
		String znode = "/test_node";
		String filename = "output";
		String exec[] = {"ls", "/home"};

		try {
			new Executor(znode, filename, exec).run();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void process(WatchedEvent event) {
		dm.process(event);
	}

	public void run() {
		try {
			synchronized(this) {
				while (!dm.dead) {
					wait();
				}
			}
		} catch (InterruptedException e) {
		}
	}

	public void closing(int rc) {
		synchronized(this) {
			notifyAll();
		}
	}

	static class StreamWriter extends Thread {
		OutputStream os;
		InputStream is;
		StreamWriter(InputStream is, OutputStream os) {
			this.is = is;
			this.os = os;
			start();
		}

		public void run() {
			byte b[] = new byte[80];
			int rc;
			try {
				while ((rc = is.read(b)) > 0) {
					os.write(b, 0, rc);
				}
			} catch (IOException e) {
			}
		}
	}

	public void exists(byte[] data) {
		if (data == null) {
			if (child != null) {
				System.out.println("Killing process");
				child.destroy();
				try {
					child.waitFor();
				} catch (InterruptedException e) {
				}
			}
			child = null;
		} else {
			if (child != null) {
				System.out.println("Stopping child");
				child.destroy();
				try {
					child.waitFor();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			try {
				FileOutputStream fos = new FileOutputStream(filename);
				fos.write(data);
				fos.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			try {
				System.out.println("Starting child");
				child = Runtime.getRuntime().exec(exec);
				new StreamWriter(child.getInputStream(), System.out);
				new StreamWriter(child.getErrorStream(), System.err);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}

