--- dhcp6c_script.c.orig	2017-02-28 19:06:15 UTC
+++ dhcp6c_script.c
@@ -58,6 +58,7 @@
 
 #include "dhcp6.h"
 #include "config.h"
+#include "dhcp6c.h"
 #include "common.h"
 
 static char sipserver_str[] = "new_sip_servers";
@@ -71,7 +72,10 @@ static char bcmcsname_str[] = "new_bcmcs_name";
 static char nispname_str[] = "new_nisp_name";
 static char bcmcsserver_str[] = "new_bcmcs_servers";
 static char bcmcsname_str[] = "new_bcmcs_name";
+static char raw_dhcp_option_str[] = "raw_dhcp_option";
 
+int client6_script(char *, int, struct dhcp6_optinfo *);
+
 int
 client6_script(scriptpath, state, optinfo)
 	char *scriptpath;
@@ -83,11 +87,15 @@ client6_script(scriptpath, state, optinfo)
 	int nisservers, nisnamelen;
 	int nispservers, nispnamelen;
 	int bcmcsservers, bcmcsnamelen;
+	int prefixes;
 	char **envp, *s;
 	char reason[32];
+	char prefixinfo[32] = "\0";
 	struct dhcp6_listval *v;
 	struct dhcp6_event ev;
+	struct rawoption *rawop;
 	pid_t pid, wpid;
+	struct dhcp6_listval *iav, *siav;
 
 	/* if a script is not specified, do nothing */
 	if (scriptpath == NULL || strlen(scriptpath) == 0)
@@ -108,6 +116,7 @@ client6_script(scriptpath, state, optinfo)
 	nispnamelen = 0;
 	bcmcsservers = 0;
 	bcmcsnamelen = 0;
+	prefixes = 0;
 	envc = 2;     /* we at least include the reason and the terminator */
 	if (state == DHCP6S_EXIT)
 		goto setenv;
@@ -160,6 +169,15 @@ client6_script(scriptpath, state, optinfo)
 	}
 	envc += bcmcsnamelen ? 1 : 0;
 
+	for (iav = TAILQ_FIRST(&optinfo->iapd_list); iav; iav = TAILQ_NEXT(iav, link)) {
+		for (siav = TAILQ_FIRST(&iav->sublist); siav; siav = TAILQ_NEXT(siav, link)) {
+			if (siav->type == DHCP6_LISTVAL_PREFIX6) {
+				prefixes += 1;
+			}
+		}
+	}
+	envc += prefixes ? 1 : 0;
+
 setenv:
 	/* allocate an environments array */
 	if ((envp = malloc(sizeof (char *) * envc)) == NULL) {
@@ -390,6 +408,60 @@ setenv:
 			strlcat(s, v->val_vbuf.dv_buf, elen);
 			strlcat(s, " ", elen);
 		}
+	}
+
+	if (prefixes) {
+#define PDINFO_MAX	64
+		char *str = "PDINFO";
+		elen = sizeof (str) + PDINFO_MAX * prefixes + 1;
+		if ((s = envp[i++] = malloc(elen)) == NULL) {
+			d_printf(LOG_NOTICE, FNAME,
+			    "failed to allocate prefixinfo strings");
+			ret = -1;
+			goto clean;
+		}
+		memset(s, 0, elen);
+		snprintf(s, elen, "%s=", str);
+		for (iav = TAILQ_FIRST(&optinfo->iapd_list); iav; iav = TAILQ_NEXT(iav, link)) {
+			for (siav = TAILQ_FIRST(&iav->sublist); siav; siav = TAILQ_NEXT(siav, link)) {
+				if (siav->type == DHCP6_LISTVAL_PREFIX6) {
+					char prefixinfo[PDINFO_MAX];
+
+					snprintf(prefixinfo, sizeof(prefixinfo),
+					    "%s/%d", in6addr2str(&siav->val_prefix6.addr, 0),
+					    siav->val_prefix6.plen);
+
+					strlcat(s, prefixinfo, elen);
+					strlcat(s, " ", elen);
+				}
+			}
+		}
+	}
+
+	/* XXX */
+	for (rawop = TAILQ_FIRST(&optinfo->rawops); rawop; rawop = TAILQ_NEXT(rawop, link)) {
+		// max of 5 numbers after last underscore (seems like max DHCPv6 option could be 65535)
+		elen = sizeof(raw_dhcp_option_str) + 1 /* underscore */ + 1 /* equals sign */ + 5;
+		elen += rawop->datalen * 2;
+		if ((s = envp[i++] = malloc(elen)) == NULL) {
+			d_printf(LOG_NOTICE, FNAME,
+			    "failed to allocate string for DHCPv6 option %d",
+			    rawop->opnum);
+			ret = -1;
+			goto clean;
+		}
+
+		// make raw options available as raw_dhcp_option_xyz=hexresponse
+		snprintf(s, elen, "%s_%d=", raw_dhcp_option_str, rawop->opnum);
+		const char * hex = "0123456789abcdef";
+		char * val = (char*)malloc(3);
+		for (int o = 0; o < rawop->datalen; o++) {
+			val[0] = hex[(rawop->data[o]>>4) & 0x0F];
+			val[1] = hex[(rawop->data[o]   ) & 0x0F];
+			val[2] = 0x00;
+			strlcat(s, val, 1);
+		}
+		free(val);
 	}
 launch:
 	/* launch the script */
