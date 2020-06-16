"""Lamport's bakery test"""
import threading

ThreadID = threading.get_ident


class BakeryLock:
    def __init__(self, n: int):
        self.flag = [False] * n  # List of bools
        self.label = [0] * n  # List of integers

    def lock(self):
        tid = ThreadID()
        self.flag[tid] = True
        self.label[tid] = max(self.label) + 1  # take ticket
        # while (EXIST k≠tid: flag[k] && (label[k],k)<<(label[i],i)) {} ...JAVA
        while any(self.flag[:tid] + self.flag[tid + 1:]) and (
                any([lab < self.label[tid] for lab in self.label])
                or any([lab == self.label[tid] for lab in self.label[:tid]])):
            pass
        print(f"Lock from: {tid}")

    def unlock(self):
        tid = ThreadID()
        self.flag[tid] = False
        print(f"Unlock from: {tid}")
