#!/bin/sh
dir_type=0
P_LIST=QGSP_BIC_HP
for job_number in {1..20..1} 
do
	echo ${job_number}
	cd /pbs/home/l/lmanzani/ProtoDUNE-VD/build/
	FILE_MAC="PNS_mac_${job_number}.mac"
	/bin/cat <<EOM >$FILE_MAC

/ULALAP/det/setOutputDirectory /sps/lbno/lmanzani/ProtoDUNE/
/ULALAP/det/setSetupName ${P_LIST}_file_${job_number}_dir_${dir_type}_200mrad
/process/had/particle_hp/use_photo_evaporation true
/process/had/particle_hp/do_not_adjust_final_state false
/process/had/particle_hp/skip_missing_isotopes true

#direction type, if 1 isotropic
/ULALAP/gun/sourceDirectionType ${dir_type}

#direction of the source
/ULALAP/gun/direction 0 1 0

#position of the center of the source in cm for x y z
/ULALAP/gun/position -75 -600 150

#choise of the source
/ULALAP/gun/sourceType 8
/ULALAP/gun/sourceEnergy 2.45 MeV

#geometry of the source
/ULALAP/gun/shape Box

#size of the box in cm for x y z
/ULALAP/gun/BoxXYZ 1 1 1

#number of events
/run/beamOn 500000


EOM
	FILEJOB="job_PNS_${job_number}.sh"
	
        /bin/cat <<EOM >$FILEJOB
#!/bin/bash
#! /bin/bash

#SBATCH --job-name=multicpu_jobs       # Job name
#SBATCH --output=multicpu_jobs_%j.log  # Standard output and error log

#SBATCH --partition=htc                # Partition choice (htc by default)

#SBATCH --ntasks=1                     # Run up to two tasks
#SBATCH --time=3:00:00              # Max time limit = 7 days

#SBATCH --mem-per-cpu=1000             # Allocate 1G of memeory per core
#SBATCH --cpus-per-task=2              # Allocate 4 cores per task, i.e 8 cores in total in this example (ntasks x cpus-per-task)

#SBATCH --mail-user=manzanilla@lapp.in2p3.fr    # Where to send mail
#SBATCH --mail-type=NONE          # Mail events (NONE, BEGIN, END, FAIL, ALL)

#SBATCH --licenses=sps                 # Declaration of storage and/or software resources

source /pbs/home/l/lmanzani/setup_ulalap.sh
cd /pbs/home/l/lmanzani/ProtoDUNE-VD/build/
./ULALAP -m ${FILE_MAC} -r ${job_number} -p ${P_LIST}
exit 0
EOM
sbatch $FILEJOB

done

