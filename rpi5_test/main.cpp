#include <iostream>
#include <thread>

int main()
{
  std::cout << "Hello from Raspberry Pi 5 ARM64\n";
  std::cout << "CPU threads: " << std::thread::hardware_concurrency() << "\n";
  return 0;
}