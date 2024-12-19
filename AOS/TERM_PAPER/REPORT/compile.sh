#!/bin/bash

# Compile the LaTeX file with pdflatex
pdflatex -interaction=nonstopmode main.tex

# Run BibTeX if references are used (optional)
bibtex main

# Re-compile to ensure references and citations are updated
pdflatex -interaction=nonstopmode main.tex
pdflatex -interaction=nonstopmode main.tex

# Clean up auxiliary files
rm -f *.aux *.log *.bbl *.blg *.toc *.out
