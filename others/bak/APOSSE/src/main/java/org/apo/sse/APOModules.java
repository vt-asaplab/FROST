/** * Copyright (C) 2017 Guoxing Chen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//***********************************************************************************************//

// This file contains APOModules
//***********************************************************************************************//

package org.apo.sse;

import com.backblaze.erasure.*;
import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Multimap;

import java.io.*;
import java.nio.ByteBuffer;
import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicInteger;

public class APOModules {
    private static int m = 1;
    private static int k = 1;
    private static double p = 0.895328521728516;
	private static double q = 0.0210561086316476;
    public static final int BYTES_IN_INT = 4;
	public static final Set<String> stopwords = Set.of("a", "about", "above", "across", "after",
		"afterwards", "again", "against", "all", "almost", "alone", "along",
		"already", "also", "although", "always", "am", "among", "amongst", "amoungst",
		"amount", "an", "and", "another", "any", "anyhow", "anyone", "anything", "anyway",
		"anywhere", "are", "around", "as", "at", "back", "be", "became", "because", "become",
		"becomes", "becoming", "been", "before", "beforehand", "behind", "being", "below",
		"beside", "besides", "between", "beyond", "bill", "both", "bottom", "but", "by",
		"call", "can", "cannot", "cant", "co", "con", "could", "couldnt", "cry", "de",
		"describe", "detail", "do", "done", "down", "due", "during", "each", "eg", "eight",
		"either", "eleven", "else", "elsewhere", "empty", "enough", "etc", "even", "ever",
		"every", "everyone", "everything", "everywhere", "except", "few", "fifteen", "fify",
		"fill", "find", "fire", "first", "five", "for", "former", "formerly", "forty", "found",
		"four", "from", "front", "full", "further", "get", "give", "go", "had", "has", "hasnt",
		"have", "he", "hence", "her", "here", "hereafter", "hereby", "herein", "hereupon",
		"hers", "herself", "him", "himself", "his", "how", "however", "hundred", "ie", "if",
		"in", "inc", "indeed", "interest", "into", "is", "it", "its", "itself", "keep", "last",
		"latter", "latterly", "least", "less", "ltd", "made", "many", "may", "me", "meanwhile",
		"might", "mill", "mine", "more", "moreover", "most", "mostly", "move", "much", "must",
		"my", "myself", "name", "namely", "neither", "never", "nevertheless", "next", "nine",
		"no", "nobody", "none", "noone", "nor", "not", "nothing", "now", "nowhere", "of", "off",
		"often", "on", "once", "one", "only", "onto", "or", "other", "others", "otherwise",
		"our", "ours", "ourselves", "out", "over", "own", "part", "per", "perhaps", "please",
		"put", "rather", "re", "same", "see", "seem", "seemed", "seeming", "seems", "serious",
		"several", "she", "should", "show", "side", "since", "sincere", "six", "sixty", "so",
		"some", "somehow", "someone", "something", "sometime", "sometimes", "somewhere",
		"still", "such", "system", "take", "ten", "than", "that", "the", "their", "them",
		"themselves", "then", "thence", "there", "thereafter", "thereby", "therefore",
		"therein", "thereupon", "these", "they", "thickv", "thin", "third", "this", "those",
		"though", "three", "through", "throughout", "thru", "thus", "to", "together", "too",
		"top", "toward", "towards", "twelve", "twenty", "two", "un", "under", "until", "up",
		"upon", "us", "very", "via", "was", "we", "well", "were", "what", "whatever", "when",
		"whence", "whenever", "where", "whereafter", "whereas", "whereby", "wherein",
		"whereupon", "wherever", "whether", "which", "while", "whither", "who", "whoever",
		"whole", "whom", "whose", "why", "will", "with", "within", "without", "would", "yet",
		"you", "your", "yours", "yourself", "yourselves"); 
	
	public static void setParameters(int m, int k, double p, double q) {
		APOModules.m = m;
		APOModules.k = k;
		APOModules.p = p;
		APOModules.q = q;
	}

	public static void setParameters(double p, double q) {
		APOModules.p = p;
		APOModules.q = q;
	}
	
	public static Multimap<String, String> getTopCommonKeywods(Multimap<String, String> originalKeywordLists, int k) {
        Multimap<String, String> topCommonKeywords = ArrayListMultimap.create();
	    Map<String, Integer> map = new TreeMap<String, Integer>();
	    for (String key : originalKeywordLists.keySet()) {
	        map.put(key, originalKeywordLists.get(key).size());
        }
        List<Map.Entry<String, Integer>> list = new ArrayList<Map.Entry<String, Integer>>(map.entrySet());
        Collections.sort(list, new Comparator<Map.Entry<String, Integer>>() {

            public int compare(Map.Entry<String, Integer> o1,
                               Map.Entry<String, Integer> o2) {
                return o2.getValue() - o1.getValue();
            }

        });
        for (Map.Entry<String, Integer> entry : list) {
            topCommonKeywords.putAll(entry.getKey(), originalKeywordLists.get(entry.getKey()));
            if (k-- == 0) break;
        }
        return topCommonKeywords;
    }

	/*
	public static Multimap<String, String> obfuscateKeywordLists(
        Multimap<String, String> keywordLists,
        ArrayList<File> listOfFile)
        throws InterruptedException, ExecutionException {

        List<String> listOfKeyword = keywordLists.keySet().stream()
                .filter(keyword -> keyword.length() >= 4 && keyword.length() <= 20)
                .filter(keyword -> !stopwords.contains(keyword.toLowerCase()))
                .filter(keyword -> keyword.matches("[a-zA-Z]+"))
                .collect(Collectors.toList());
        // Collections.shuffle(listOfKeyword, new Random());
        // listOfKeyword = listOfKeyword.subList(0, Math.min(65536, listOfKeyword.size()));
        
        AtomicInteger count = new AtomicInteger(0);
        long total = listOfKeyword.size();

        int numThreads = Runtime.getRuntime().availableProcessors();
        ExecutorService pool = Executors.newFixedThreadPool(numThreads);

        List<Future<Multimap<String, String>>> futures = new ArrayList<>();

        for (String keyword : listOfKeyword) {
            futures.add(pool.submit(() -> {
                long c = count.incrementAndGet();
                if (c % 100 == 0) {
                    System.out.printf("Progress: %.2f%% of keywords %n",
                            (c * 100.0) / total);
                }

                Multimap<String, String> localMap = HashMultimap.create();
                ThreadLocalRandom rng = ThreadLocalRandom.current();

                for (File file : listOfFile) {
                    if (keywordLists.get(keyword).contains(file.getName())) {
                        for (int i = 0; i < m; i++) {
                            if (rng.nextDouble() < p) {
                                localMap.put(keyword.toLowerCase(), file.getName() + "." + i);
                            }
                        }
                    } else {
                        for (int i = 0; i < m; i++) {
                            if (rng.nextDouble() < q) {
                                localMap.put(keyword.toLowerCase(), file.getName() + "." + i);
                            }
                        }
                    }
                }
                return localMap;
            }));
        }

        // Collect results
        Multimap<String, String> result = HashMultimap.create();
        for (Future<Multimap<String, String>> future : futures) {
            result.putAll(future.get()); // waits for each task
        }

        pool.shutdown();
        return result;
    }
	*/
	
	public static Multimap<String, String> obfuscateKeywordLists(Multimap<String, String> keywordLists, ArrayList<File> listOfFile) {
		Multimap<String, String> obfuscatedKeywordLists = HashMultimap.create();
		Random rng = new Random();

		// List<String> listOfKeyword = new ArrayList<String>(keywordLists.keySet());
		List<String> listOfKeyword = keywordLists.keySet().stream()
                .filter(keyword -> keyword.length() >= 4 && keyword.length() <= 20)
                .filter(keyword -> !stopwords.contains(keyword.toLowerCase()))
                .filter(keyword -> keyword.matches("[a-zA-Z]+"))
                .collect(Collectors.toList());
		
		int total = listOfKeyword.size();
        int count = 0;
		
		for (String keyword : listOfKeyword) {
			count++;
            System.out.printf("Progress: %.2f%% of keywords %n", (count * 100.0) / total);
			
			for (File file : listOfFile) {
				if (keywordLists.get(keyword).contains(file.getName())) {
					for (int i = 0; i < m; i++) {
						if (rng.nextDouble() < p) {
							obfuscatedKeywordLists.put(keyword.toLowerCase(), file.getName() + "." + i);
						}
					}
				} else {
					for (int i = 0; i < m; i++) {
						if (rng.nextDouble() < q) {
							obfuscatedKeywordLists.put(keyword.toLowerCase(), file.getName() + "." + i);
						}
					}
				}
			}
		}
		return obfuscatedKeywordLists;
	}

	public static void erasureCodeEncoding(List<File> listOfFile, String originalPathName, String shardsPathName) throws IOException {
	    for (File file : listOfFile) {
	        encodeOneFile(file, originalPathName, shardsPathName);
        }

    }

	public static List<String> erasureCodeDecoding(List<String> listOfFile, String shardsPathName, String resultsPathName) throws IOException {
        List<String> listOfDecodedFile = new ArrayList<String>();

        Multimap<String, Byte> shardMap = HashMultimap.create();

        for (String fileName : listOfFile) {
            int i = fileName.lastIndexOf('.');
            shardMap.put(fileName.substring(0, i), Byte.valueOf(fileName.substring(i + 1)));

        }

        for (String fileName : shardMap.keySet()) {
            if (shardMap.get(fileName).size() >= k) {
                listOfDecodedFile.add(fileName + shardMap.get(fileName));
                decodeOneFile(fileName, shardMap.get(fileName), shardsPathName, resultsPathName);
            }
        }

        return listOfDecodedFile;
    }

    public static void encodeOneFile(File inputFile, String originalPathName, String shardsPathName) throws IOException {
	    int DATA_SHARDS = k;
	    int PARITY_SHARDS = m - k;
	    int TOTAL_SHARDS = m;

        final int fileSize = (int) inputFile.length();

        // Figure out how big each shard will be.  The total size stored
        // will be the file size (8 bytes) plus the file.
        final int storedSize = fileSize + BYTES_IN_INT;
        final int shardSize = (storedSize + DATA_SHARDS - 1) / DATA_SHARDS;

        // Create a buffer holding the file size, followed by
        // the contents of the file.
        final int bufferSize = shardSize * DATA_SHARDS;
        final byte [] allBytes = new byte[bufferSize];
        ByteBuffer.wrap(allBytes).putInt(fileSize);
        InputStream in = new FileInputStream(inputFile);
        int bytesRead = in.read(allBytes, BYTES_IN_INT, fileSize);
        if (bytesRead != fileSize) {
            throw new IOException("not enough bytes read");
        }
        in.close();

        // Make the buffers to hold the shards.
        byte [] [] shards = new byte [TOTAL_SHARDS] [shardSize];

        // Fill in the data shards
        for (int i = 0; i < DATA_SHARDS; i++) {
            System.arraycopy(allBytes, i * shardSize, shards[i], 0, shardSize);
        }

        // Use Reed-Solomon to calculate the parity.
        ReedSolomon reedSolomon = ReedSolomon.create(DATA_SHARDS, PARITY_SHARDS);
        reedSolomon.encodeParity(shards, 0, shardSize);

        // Write out the resulting files.
        for (int i = 0; i < TOTAL_SHARDS; i++) {
            File outputFile = new File(shardsPathName + inputFile.getAbsolutePath().substring(originalPathName.length()) + "." + i);
            outputFile.getParentFile().mkdirs();
            OutputStream out = new FileOutputStream(outputFile);
            out.write(shards[i]);
            out.close();
        }
    }

    public static void decodeOneFile(String inputFile, Collection<Byte> shardIndexes, String shardsPathName, String resultsPathName) throws IOException {
        int DATA_SHARDS = k;
        int PARITY_SHARDS = m - k;
        int TOTAL_SHARDS = m;

        // Read in any of the shards that are present.
        // (There should be checking here to make sure the input
        // shards are the same size, but there isn't.)
        final byte [] [] shards = new byte [TOTAL_SHARDS] [];
        final boolean [] shardPresent = new boolean [TOTAL_SHARDS];
        int shardSize = 0;
        int shardCount = 0;
        for (Byte i : shardIndexes) {
            File shardFile = new File(
                    shardsPathName,
                    inputFile + "." + i);
            if (shardFile.exists()) {
                shardSize = (int) shardFile.length();
                shards[i] = new byte [shardSize];
                shardPresent[i] = true;
                shardCount += 1;
                InputStream in = new FileInputStream(shardFile);
                in.read(shards[i], 0, shardSize);
                in.close();
            }
        }

        // We need at least DATA_SHARDS to be able to reconstruct the file.
        if (shardCount < DATA_SHARDS) {
            System.out.println("Not enough shards present for " + inputFile);
            return;
        }

        // Make empty buffers for the missing shards.
        for (int i = 0; i < TOTAL_SHARDS; i++) {
            if (!shardPresent[i]) {
                shards[i] = new byte [shardSize];
            }
        }

        // Use Reed-Solomon to fill in the missing shards
        ReedSolomon reedSolomon = ReedSolomon.create(DATA_SHARDS, PARITY_SHARDS);
        reedSolomon.decodeMissing(shards, shardPresent, 0, shardSize);

        // Combine the data shards into one buffer for convenience.
        // (This is not efficient, but it is convenient.)
        byte [] allBytes = new byte [shardSize * DATA_SHARDS];
        for (int i = 0; i < DATA_SHARDS; i++) {
            System.arraycopy(shards[i], 0, allBytes, shardSize * i, shardSize);
        }

        // Extract the file length
        int fileSize = ByteBuffer.wrap(allBytes).getInt();

        // Write the decoded file
        File decodedFile = new File(resultsPathName, inputFile);
        OutputStream out = new FileOutputStream(decodedFile);
        out.write(allBytes, BYTES_IN_INT, fileSize);
    }
}
