CREATE TABLE `webmoney_pays_ext` ( `pay_id` int(10) unsigned NOT NULL, `session` int(32) NOT NULL AUTO_INCREMENT, `stage` tinyint(4) unsigned NOT NULL, `kiosk` int(32) unsigned NOT NULL, `errmsg` mediumtext NOT NULL, PRIMARY KEY (`pay_id`), KEY `stage` (`stage`), KEY `session` (`session`) )ENGINE=MyISAM AUTO_INCREMENT=100000 DEFAULT CHARSET=utf8;
