# fragment_extractor

### Synopsis
Tool to extract read counts from cfDNA samples.

Features:
- Count total read count per window (default 1Mb)
- Count individual short fragments (< 150bp) and long fragments (> 150bp)
- fragment size ratio (short/long)

Outputs:
- wig file (can be fed to ichorCNA!)
- bed file with the following structure:
```
chr	pos	end	read_count	ultra_short_fragments	short_fragments	long_fragments	fragment_size_ratio
chr1	0	1000000	20105	290	3974	15841	0.0729743
chr1	1000000	2000000	73446	1322	16566	55558	0.079802
chr1	2000000	3000000	58489	1032	13101	44356	0.0787726
```
### Usage
```
./fragment_extractor <input.bam> <input.windows.bed> <output.bed> output.wig>
```

### Todo:
- Include a parameter to control windo size.
- Define short, long fragment thresholds.
