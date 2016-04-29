import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.ZooDefs.Ids;
import org.apache.zookeeper.data.Stat;
import org.apache.zookeeper.CreateMode;

public class ZooKeeperOperator extends AbstractZooKeeper {
	private static Log log = LogFactory.getLog(AbstractZooKeeper.class.getName());

	public void create(String path, byte[] data) throws KeeperException, InterruptedException {
		this.zooKeeper.create(path, data, Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT_SEQUENTIAL);
	}

	public void getChild(String path) throws KeeperException, InterruptedException {
		try {
			List<String> list = this.zooKeeper.getChildren(path, false);
			if (list.isEmpty()) {
				log.info(path + "中没有结点");
			} else {
				log.info(path + "中存在结点");
				for (String child:list) {
					log.info("节点为：" + child);
				}
			}
		} catch (KeeperException.NoNodeException e) {
			throw e;
		}
	}


	public byte[] getData(String path) throws KeeperException, InterruptedException {
		return this.zooKeeper.getData(path, true, null);
	}

	public void setData(String path, String value) throws KeeperException, InterruptedException {
		Stat stat =  this.zooKeeper.setData(path, value.getBytes(), -1);
		log.info("stat after set: " + stat);
	}

	private static void waitForExit()
	{
		try {
			char i = (char)System.in.read();
			if (i == 'q')
			{
				return;
			}
			waitForExit();
		} catch(Exception e) {
			e.printStackTrace();
		}
	}

	public static void main(String[] args) {
		try {
			ZooKeeperOperator zkOperator = new ZooKeeperOperator();
			zkOperator.connect("192.168.21.241:2181,192.168.21.241:2182,192.168.21.241:2183");

			zkOperator.findLeader(zkOperator);

			String zkTest = "ZooKeeper Java API test";
			zkOperator.create("/hello/child3", zkTest.getBytes());
			log.info("获取设置的信息:" + new String(zkOperator.getData("/hello/child3")));
			System.out.println("节点孩子信息:");
			zkOperator.getChild("/hello");
			zkOperator.setData("/hello/child3", "New third child!");

			waitForExit();
			
			zkOperator.close();
		} catch(Exception e) {
			e.printStackTrace();
		}
	}
}
