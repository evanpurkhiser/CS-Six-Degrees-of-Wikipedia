all: titles-sorted links-simple-sorted

%:
	wget http://users.on.net/~henry/pagerank/$@.zip
	unzip $@.zip
	rm $@.zip
	mv $@.txt $@
