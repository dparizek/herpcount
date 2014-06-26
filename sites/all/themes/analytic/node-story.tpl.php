<?php
// $Id: node.tpl.php,v 1.3 2008/12/25 13:33:29 troy Exp $
?>
  <div class="node<?php if (!$status) { print " node-unpublished"; } ?>">


    <?php if (!$teaser) { ?><h1 class="title"><a href="<?php print $node_url?>"><?php print $title?></a></h1><?php }; ?>
   
    
    <div class="content clear-block"><?php print $content?></div>
	
  </div>
