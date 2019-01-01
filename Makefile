diet: src/diet.c
	cc -D_GNU_SOURCE -Wall src/diet.c -o diet

clean:
	rm -f diet
