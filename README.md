# LakeComo
Simulation model of Lake Como that can be combined with the [Borg Multi-Objective Evolutionary Algorithm](http://borgmoea.org/) or another MOEA implemented in the [MOEA Framework](http://moeaframework.org/) to design a set of Pareto optimal operating policies via [Evolutionary Multi-Objective Direct Policy Search](https://ascelibrary.org/doi/abs/10.1061/(ASCE)WR.1943-5452.0000570) policy design. 

Lake Como is a sub-alpine lake in the Italian lake district, northern Italy. It is the third-largest lake in Italy and reaches a maximum depth of 410 m, which makes it the fifth-deepest lake in Europe. The lake is shaped like an inverted 'Y' surrounded by mountains and primarily fed by the Adda River, which also serves as the only exit point of the lake. The lake catchment has an area of 4,733 km2, with approximately 90% in Italy and 10% in Switzerland. The hydrologic regime is snow-rainfall dominated, characterized by relatively dry winters and summers and high peaks of flow in spring and autumn, due to snowmelt and rainfall, respectively.
The Lake Como system involves numerous economic activities and has been actively studied since the 1980s. The existing regulation of the lake is driven by two primary, competing objectives: water supply mainly for irrigation, and flood control in the city of Como, which sits at the lowest point on the lake shoreline. Over the last few years, the increasing frequency and intensity of severe droughts emphasized the importance of additional, so far marginalized services provided by the lake operations, such as preventing low levels from supporting recreational activities and ecosystem preservation downstream of the lake. These four objectives are formulated as reliability indicators, all to be maximized. In addition, we added a fifth objective accounting for equity and formulated as the ratio between the standard derivation and mean of the performance in the four objectives.

To compile and run:
* Run `make` in the test folder to compile
* Run `./LakeComoSim settings_lakeComo_equity.txt < ../data/u_29.txt` to perform a simulation with a random policy 

The simulation is performed using a 1 year trajectory of inflow data simulated by a hydrologic model; the mass balance equation of the lake storage is integrated over time with a hourly time step; the daily release decision is determined by a Gaussian Radial Basis Function with 3 inputs (i.e. sine and cosine of the day of the year, lake level), 1 output, and 4 Gaussian bases, for a total of 29 policy parameters (decision variables). See the settings files in the test folder for further details.
The observed inflows used in the paper are from [Consorzio dell'Adda](http://www.addaconsorzio.it/). 
The full set of results and the code to reproduce the figures of the paper are available [HERE](https://polimi365-my.sharepoint.com/:f:/g/personal/10821937_polimi_it/ErEcif74xMVAtKXh9i1PScQBJBlMRnP98h695sQj1wPwtg?e=XGQ10P). 


**References:**

Yang, G., M. Giuliani, and A. Castelletti (2022), [Operationalizing equity in multipurpose water systems](https://hess.copernicus.org/preprints/hess-2022-99/), Hydrology and Earth System Sciences.

----
**Copyright:**
  
Copyright 2021 [Environmental Intelligence Lab](https://www.ei.deib.polimi.it/) - Politecnico di Milano.
  
Developers: Matteo Giuliani and Andrea Castelletti.
  
LakeComo is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
  
The code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
  
You should have received a copy of the GNU General Public License along with LakeComo.  If not, see <http://www.gnu.org/licenses/licenses.en.html>.
