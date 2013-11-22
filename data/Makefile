all: page-titles page-links

download/enwiki-latest-%.sql:
	mkdir -p download
	wget -O $@ http://dumps.wikimedia.org/enwiki/latest/enwiki-latest-$*.sql.gz
	gunzup $@.gz

# Normalize page titles
page-titles: download/enwiki-latest-page.sql
	./scripts/extract-titles download/enwiki-latest-page.sql $@

# Normalize page links
# Page titles must first be normalize as we need this data to get the page ids
# for each of the linked pages
page-links: download/enwiki-latest-pagelinks.sql page-titles
	./scripts/normalize-pagelinks download/enwiki-latest-pagelinks.sql page-titles $@

# Download required files
download: download/enwiki-latest-page.sql download/enwiki-latest-pagelinks.sql

