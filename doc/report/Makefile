%.pdf: %.md
	pandoc $*.md -V links-as-notes -V geometry:margin=1in --template=template.tex -o $*.pdf

all: report.pdf
