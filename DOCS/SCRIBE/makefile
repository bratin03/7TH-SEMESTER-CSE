# Define the source and target files
TEX_FILE_NAME = JIT.tex
PDF_FILE_NAME = JIT_Part_1.pdf

# Default target
all: $(PDF_FILE_NAME)

# Rule to build the PDF from the TeX file
$(PDF_FILE_NAME): $(TEX_FILE_NAME)
	pdflatex -jobname=$(basename $(PDF_FILE_NAME)) $(TEX_FILE_NAME)

# Clean rule to remove generated files
clean:
	rm -f *.aux *.log *.out $(PDF_FILE_NAME)
