import java.util.Random;

public class BarrierTest {
	private static final String addr = "192.168.21.241:2181,192.168.21.241:2182,192.168.21.241:2183";

	private static final int threadsNum = 3;

	public static void main(String args[]) throws Exception {
		for (int i = 0; i < threadsNum; ++i) {
			Process p = new Process(i, new Barrier(addr, "/barrier", threadsNum));
			p.start();
		}
	}
}

class Process extends Thread {
	private String name;
	private Barrier barrier;
	private int id;

	public Process(int id, Barrier barrier) {
		this.name = "Thread-" + id;
		this.barrier = barrier;
		this.id = id;
	}

	@Override
	public void run() {
		try {
			barrier.enter(name);
			System.out.println(name + " enter");
			//Thread.sleep(1000 + new Random().nextInt(2000));
			Thread.sleep(id * 1000);
			barrier.leave(name);
			System.out.println(name + " leave");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
