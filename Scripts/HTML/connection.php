<?php
function connected($instring){
        if($instring[0] == "O"){
                echo "<font color='green'>Connected</font>";
        }
        else{
                echo "<font color='red'>No Connection</font>";
        }
}
$data_reel = shell_exec("sudo python data/data-reel.py i 0 &");
$hose_reel = shell_exec("sudo python hose/hose-reel.py i 0 &");
$pump = shell_exec("sudo python hose/valve.py i 0 &");
?>
    <div style="float:left">
        Hose Reel: <?php connected($hose_reel); ?><br>
        Pump/Valve: <?php connected($pump); ?>
    </div>
    <div style="float:right; align:right;">

        Data Reel: <?php connected($data_reel); ?><br>
        Sampler: <?php connected($sampler); ?>
    </div>
