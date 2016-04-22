import java.util.Random;

public class BarrierTest {

	public static void main(String args[]) throws Exception {
		for (int i = 0; i < 3; ++i) {
			Process p = new Process("Thread-" + i, new Barrier("/test_node", 3));
			p.start();
		}
	}
}

class Process extends Thread {
	private String name;
	private Barrier barrier;

	public Process(String name, Barrier barrier) {
		this.name = name;
		this.barrier = barrier;
	}

	@Override
	public void run() {
		try {
			barrier.enter(name);
			System.out.println(name + " enter");
			Thread.sleep(1000 + new Random().nextInt(2000));
			barrier.leave(name);
			System.out.println(name + " leave");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
					
