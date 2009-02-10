all:
	mscgen -T png -i network_sequence.txt -o network_sequence.png
	display network_sequence.png &
	dot -Tpng network_states.dot  | display - &

