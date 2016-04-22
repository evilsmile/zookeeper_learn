import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.ZooDefs.Ids;


public class ZooKeeperOperator extends AbstractZooKeeper {
	private static Log log = LogFactory.getLog(AbstractZooKeeper.class.getName());

	public void create(String path, byte[] data) throws KeeperException, InterruptedException {
		this.zooKeeper.create(path, data, Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT);
	}

	public void getChild(String path) throws KeeperException, InterruptedException {
		try {
			List<String> list = this.zooKeeper.getChildren(path, false);
			if (list.isEmpty()) {
				log.debug(path + "中没有结点");
			} else {
				log.debug(path + "中存在结点");
				for (String child:list) {
					log.debug("节点为：" + child);
				}
			}
		} catch (KeeperException.NoNodeException e) {
			throw e;
		}
	}

	public byte[] getData(String path) throws KeeperException, InterruptedException {
		return this.zooKeeper.getData(path, false, null);
	}

	public static void main(String[] args) {
		try {
			ZooKeeperOperator zkOperator = new ZooKeeperOperator();
			zkOperator.connect("192.168.21.241:2181,192.168.21.241:2182,192.168.21.241:2183");
			byte[] data = new byte[]{'a', 'b', 'c', 'd'};

			String zkTest = "ZooKeeper Java API test";
		//	zkOperator.create("/hello/child3", zkTest.getBytes());
			log.debug("获取设置的信息:" + new String(zkOperator.getData("/hello/child3")));
			System.out.println("节点孩子信息:");
			zkOperator.getChild("/hello");
			zkOperator.close();
		} catch(Exception e) {
			e.printStackTrace();
		}
	}
}
