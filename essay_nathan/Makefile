essay.pdf: essay.md packages.svg classes.svg backgrounds/background1.pdf
	pandoc --template "eisvogel.tex" --listings essay.md -o essay.pdf

packages.svg classes.svg: packages.txt classes.txt
	plantuml packages.txt classes.txt -tsvg
