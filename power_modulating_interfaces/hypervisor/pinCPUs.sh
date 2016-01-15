echo "pinning cpus"
for i in $(seq 0 11); do
	sudo xl vcpu-pin Domain-0 $i $i $i
done
sudo xl vcpu-list
