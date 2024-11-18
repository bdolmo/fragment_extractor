#include <htslib/sam.h>
#include <htslib/hts.h>
#include <string>
#include <vector>
#include <tuple>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

struct FragmentStats {
    int read_count;
    int ultra_short_fragments;
    int short_fragments;
    int long_fragments;
    double fragment_size_ratio;
};

// Calculate fragment statistics for each region
FragmentStats CalculateFragmentStats(htsFile* bamFile, bam_hdr_t* header, hts_idx_t* idx, const std::string& region) {
    FragmentStats stats = {0, 0, 0, 0, 0.0};

    hts_itr_t* iter = sam_itr_querys(idx, header, region.c_str());
    if (!iter) {
        throw std::runtime_error(" ERROR: Failed to set iterator for region: " + region);
    }
    bam1_t* b = bam_init1();
    while (sam_itr_next(bamFile, iter, b) >= 0) {

        if (b->core.qual < 20) {
            continue; 
        }

        ++stats.read_count;

        const uint32_t* cigar = bam_get_cigar(b);
        int sequence_length = b->core.l_qseq;

        if (bam_cigar_op(cigar[0]) == BAM_CSOFT_CLIP) {
            sequence_length -= bam_cigar_oplen(cigar[0]);
        }
        if (bam_cigar_op(cigar[b->core.n_cigar - 1]) == BAM_CSOFT_CLIP) {
            sequence_length -= bam_cigar_oplen(cigar[b->core.n_cigar - 1]);
        }

        if (sequence_length < 100) {
            ++stats.ultra_short_fragments;
        } else if (sequence_length >= 100 && sequence_length <= 150) {
            ++stats.short_fragments;
        } else {
            ++stats.long_fragments;
        }
    }
    bam_destroy1(b);
    hts_itr_destroy(iter);

    // Calculate fragment size ratio
    if (stats.short_fragments > 0) {
        stats.fragment_size_ratio = static_cast<double>(stats.ultra_short_fragments) / stats.short_fragments;
    }

    return stats;
}

// Parse regions from BED file
std::vector<std::tuple<std::string, int, int>> ParseRegionsFromBed(const std::string& bedFile) {
    std::vector<std::tuple<std::string, int, int>> regions;
    std::ifstream infile(bedFile);

    if (!infile) {
        throw std::runtime_error(" ERROR: Failed to open BED file: " + bedFile);
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream ss(line);
        std::string chrom;
        int start, end;
        if (ss >> chrom >> start >> end) {
            regions.emplace_back(chrom, start, end);
        }
    }

    return regions;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <BAM file> <BED file> <output file> <WIG file>" << std::endl;
        return 1;
    }

    std::string bamFilePath = argv[1];
    std::string bedFile = argv[2];
    std::string outputFile = argv[3];
    std::string wigFile = argv[4];

    try {
        htsFile* bamFile = sam_open(bamFilePath.c_str(), "r");
        if (!bamFile) throw std::runtime_error(" ERROR: Failed to open BAM file: " + bamFilePath);

        bam_hdr_t* header = sam_hdr_read(bamFile);
        if (!header) throw std::runtime_error(" ERROR: Failed to read BAM header from: " + bamFilePath);

        hts_idx_t* idx = sam_index_load(bamFile, bamFilePath.c_str());
        if (!idx) throw std::runtime_error(" ERROR: Failed to load BAM index for: " + bamFilePath);

        std::vector<std::tuple<std::string, int, int>> regions = ParseRegionsFromBed(bedFile);

        std::ofstream outFile(outputFile);
        if (!outFile) throw std::runtime_error(" ERROR: Failed to open output file: " + outputFile);

        // Open the WIG output file
        std::ofstream wigOut(wigFile);
        if (!wigOut) throw std::runtime_error(" ERROR: Failed to open WIG file: " + wigFile);

        // Write headers for the main output
        outFile << "chr\tpos\tend\tread_count\tultra_short_fragments\tshort_fragments\tlong_fragments\tfragment_size_ratio\n";

        std::string current_chrom = "";
        int step = 1000000; 

        // Process each region
        for (const auto& [chrom, start, end] : regions) {
            std::string region = chrom + ":" + std::to_string(start) + "-" + std::to_string(end);
            FragmentStats stats = CalculateFragmentStats(bamFile, header, idx, region);

            // Write data to the main output file
            outFile << chrom << "\t" << start << "\t" << end << "\t" 
                    << stats.read_count << "\t" 
                    << stats.ultra_short_fragments << "\t" 
                    << stats.short_fragments << "\t" 
                    << stats.long_fragments << "\t" 
                    << stats.fragment_size_ratio << "\n";

            // Write short_fragments data to the WIG file in fixedStep format
            if (chrom != current_chrom) {
                // Start a new fixedStep section for a new chromosome
                wigOut << "fixedStep chrom=" << chrom << " start=1" << " step=" << step << " span=" << step << "\n";
                current_chrom = chrom;
            }
            wigOut << stats.short_fragments << "\n";
        }

        outFile.close();
        wigOut.close();

        // Clean up
        hts_idx_destroy(idx);
        bam_hdr_destroy(header);
        sam_close(bamFile);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


