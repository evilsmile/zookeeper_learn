import org.apache.zookeeper.KeeperException;

public class QueueTest {
	
	private static final String addr = "192.168.21.241:2181,192.168.21.241:2182,192.168.21.241:2183";

	public static void main(String args[]) {
		Queue q = new Queue(addr, "/app1");	

		Integer max = new Integer(3);

		if (args[0].equals("p")) {
			System.out.println("Producer");
			for (int i = 0; i < max; ++i) {
				try {
					q.produce(10 + i);
				} catch (KeeperException e) {
				} catch (InterruptedException e) {
				}
			}
		} else {
			System.out.println("Consume");

			for (int i = 0; i < max; ++i) {
				try {
					int r = q.consume();
					System.out.println("Item: " + r);
				} catch (KeeperException e) {
					i++;
				} catch (InterruptedException e) {
				}
			}
		}
	}
}
