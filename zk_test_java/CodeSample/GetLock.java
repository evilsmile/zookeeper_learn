void getLock() throws KeeperException, InterruptedException {
	List<String> list = zk.getChildren(root, false);
	String[] nodes = list.toArray(new String[list.size()]);
	Arrays.sort(nodes);

	if (myZnode.equals(root + "/" + nodes[0])) {
		doAction();
	} else {
		waitForLock(nodes[0]);
	}
}

void waitForLock(String lower) throws InterruptedException, KeeperException {
	// 等最小的节点退出，然后获得锁
	Stat stat = zk.exists(root + "/" + lower, true);
	if (stat != null) {
		mutex.wait();
	} else {
		getLock();
	}
}
