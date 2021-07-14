pulse_imm() {

ip=$1
ch=$2
len=$3
	
addr=0x62348
offset[0]=0x00
offset[1]=0x04
offset[2]=0x08
offset[3]=0x0c
offset[4]=0x10
	
addr=$(($addr+${offset[$ch]}))
	
eb-mem.sh --write --ip $ip --address $addr --value $len
	
addr=0x6235c
val=0xffffffff
	
mask[0]=0x01
mask[1]=0x02
mask[2]=0x04
mask[3]=0x08
mask[4]=0x10
	
val=$(($val & ${mask[$ch]}))
	
eb-mem.sh --write --ip $ip --address $addr --value $val
}

pulse_imm 10.10.10.10 1 0xffffffff

## Configure channel 1 as Output

config_ch_o() {

ip=$1
ch=$2

mask1[0]=0xfffffffc
mask1[1]=0xffffffcf
mask1[2]=0xfffffcff
mask1[3]=0xffffcfff
mask1[4]=0xfffcffff

mask2[0]=0x01
mask2[1]=0x10
mask2[2]=0x100
mask2[3]=0x1000
mask2[4]=0x10000

mask3[0]=0xfffffffb
mask3[1]=0xffffffbf
mask3[2]=0xfffffbff
mask3[3]=0xffffbfff
mask3[4]=0xfffbffff

eb-mem.sh --read --ip $ip --address 0x6233c > value.txt
value=`cat value.txt`
value=0x$value
value=$(($value & ${mask1[$ch]}))
eb-mem.sh --write --ip $ip --address 0x6233c --value $value

eb-mem.sh --read --ip $ip --address 0x6233c > value.txt
value=`cat value.txt`
value=0x$value
value=$(($value | ${mask2[$ch]}))
eb-mem.sh --write --ip $ip --address 0x6233c --value $value

eb-mem.sh --read --ip $ip --address 0x6233c > value.txt
value=`cat value.txt`
value=0x$value
value=$(($value & ${mask3[$ch]}))
eb-mem.sh --write --ip $ip --address 0x6233c --value $value

}

config_ch_o 10.10.10.10 1
