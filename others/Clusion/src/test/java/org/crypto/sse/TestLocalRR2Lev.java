/** * Copyright (C) 2016 Tarik Moataz
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
// This file is to test the 2Lev construction by Cash et al. NDSS'14. 
//**********************************************************************************************

package org.crypto.sse;

import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.security.Security;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class TestLocalRR2Lev {

	public static void main(String[] args) throws Exception {

		double tpr = 0.95;
		double fpr = 0.1;
		int n = 1024;
		String d = "./maildir";

		for (int i = 0; i < args.length; i++) {
			switch (args[i]) {
				case "-t":
					if (i + 1 < args.length) {
						tpr = Double.parseDouble(args[++i]);
					}
					break;
				case "-f":
					if (i + 1 < args.length) {
						fpr = Double.parseDouble(args[++i]);
					}
					break;
				case "-n":
					if (i + 1 < args.length) {
						n = Integer.parseInt(args[++i]);
					}
					break;
				case "-d":
                	if (i + 1 < args.length) {
                    	d = args[++i];
                	}
                	break;
				default:
					System.out.println("Warning: Unknown argument " + args[i]);
			}
		}
		
		if (Security.getProvider(BouncyCastleProvider.PROVIDER_NAME) == null) {
		    Security.addProvider(new BouncyCastleProvider());
		}

		BufferedReader keyRead = new BufferedReader(new InputStreamReader(System.in));

		// System.out.println("Enter your password :");

		// String pass = keyRead.readLine();

		String pass = "123";
		
		List<byte[]> listSK = IEX2Lev.keyGen(256, pass, "salt/salt", 100000);

		// System.out.println("Enter the relative path name of the folder that contains the files to make searchable");

		// String pathName = keyRead.readLine();

		String pathName = d;

		ArrayList<File> listOfFile = new ArrayList<File>();
		TextProc.listf(pathName, listOfFile, n);

		TextProc.TextProc(false, pathName, n);

		// The two parameters depend on the size of the dataset. Change
		// accordingly to have better search performance
		int bigBlock = 1000;
		int smallBlock = 100;
		int dataSize = 20000000;
		
		// Construction of the global multi-map
		System.out.println("\nBeginning of Encrypted Multi-map creation \n");

		RR2Lev twolev = RR2Lev.constructEMMParGMM(listSK.get(0), TextExtractPar.lp1, bigBlock, smallBlock, dataSize);

		while (true) {

			System.out.println("Enter the keyword to search for:");
			String keyword = keyRead.readLine();
			byte[][] token = RR2Lev.token(listSK.get(0), keyword);

			long startTime = System.nanoTime();

			List<String> result = twolev.query(token, twolev.getDictionary(), twolev.getArray());
			
			System.out.println("Final Result: " + result);
			
			long estimatedTime = System.nanoTime() - startTime;

			System.out.println("Execution time: " + estimatedTime + " ns");

			double bw = (result.size() * tpr * Integer.BYTES + n * fpr * Integer.BYTES + 16) / 1024.0;
			
			System.out.println("Bandwidth cost: " + bw + " KB");
		}
	}
}
