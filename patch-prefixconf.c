--- prefixconf.c.orig	2017-02-28 19:06:15 UTC
+++ prefixconf.c
@@ -57,6 +57,7 @@
 #include "config.h"
 #include "common.h"
 #include "timer.h"
+#include "dhcp6c.h"
 #include "dhcp6c_ia.h"
 #include "prefixconf.h"
 
@@ -100,23 +101,22 @@ struct dhcp6_ifprefix {
 	struct sockaddr_in6 ifaddr;
 };
 
-static struct siteprefix *find_siteprefix __P((struct siteprefix_list *,
-    struct dhcp6_prefix *, int));
-static void remove_siteprefix __P((struct siteprefix *));
-static int isvalid __P((struct iactl *));
-static u_int32_t duration __P((struct iactl *));
-static void cleanup __P((struct iactl *));
-static int renew_prefix __P((struct iactl *, struct dhcp6_ia *,
-    struct dhcp6_eventdata **, struct dhcp6_eventdata *));
-static void renew_data_free __P((struct dhcp6_eventdata *));
+static struct siteprefix *find_siteprefix(struct siteprefix_list *,
+    struct dhcp6_prefix *, int);
+static void remove_siteprefix(struct siteprefix *);
+static int isvalid(struct iactl *);
+static uint32_t duration(struct iactl *);
+static void cleanup(struct iactl *);
+static int renew_prefix(struct iactl *, struct dhcp6_ia *,
+    struct dhcp6_eventdata **, struct dhcp6_eventdata *);
+static void renew_data_free(struct dhcp6_eventdata *);
 
-static struct dhcp6_timer *siteprefix_timo __P((void *));
+static struct dhcp6_timer *siteprefix_timo(void *);
 
-static int add_ifprefix __P((struct siteprefix *,
-    struct dhcp6_prefix *, struct prefix_ifconf *));
+static int add_ifprefix(struct siteprefix *,
+    struct dhcp6_prefix *, struct prefix_ifconf *);
 
-extern struct dhcp6_timer *client6_timo __P((void *));
-static int pd_ifaddrconf __P((ifaddrconf_cmd_t, struct dhcp6_ifprefix *ifpfx));
+static int pd_ifaddrconf(ifaddrconf_cmd_t, struct dhcp6_ifprefix *ifpfx);
 
 int
 update_prefix(ia, pinfo, pifc, dhcpifp, ctlp, callback)
@@ -125,7 +125,7 @@ update_prefix(ia, pinfo, pifc, dhcpifp, ctlp, callback
 	struct pifc_list *pifc;
 	struct dhcp6_if *dhcpifp;
 	struct iactl **ctlp;
-	void (*callback)__P((struct ia *));
+	void (*callback)(struct ia *);
 {
 	struct iactl_pd *iac_pd = (struct iactl_pd *)*ctlp;
 	struct siteprefix *sp;
@@ -304,13 +304,13 @@ isvalid(iac)
 	return (1);
 }
 
-static u_int32_t
+static uint32_t
 duration(iac)
 	struct iactl *iac;
 {
 	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
 	struct siteprefix *sp;
-	u_int32_t base = DHCP6_DURATION_INFINITE, pltime, passed;
+	uint32_t base = DHCP6_DURATION_INFINITE, pltime, passed;
 	time_t now;
 
 	/* Determine the smallest period until pltime expires. */
@@ -318,7 +318,7 @@ duration(iac)
 	for (sp = TAILQ_FIRST(&iac_pd->siteprefix_head); sp;
 	    sp = TAILQ_NEXT(sp, link)) {
 		passed = now > sp->updatetime ?
-		    (u_int32_t)(now - sp->updatetime) : 0;
+		    (uint32_t)(now - sp->updatetime) : 0;
 		pltime = sp->prefix.pltime > passed ?
 		    sp->prefix.pltime - passed : 0;
 
@@ -407,7 +407,7 @@ siteprefix_timo(arg)
 {
 	struct siteprefix *sp = (struct siteprefix *)arg;
 	struct ia *ia;
-	void (*callback)__P((struct ia *));
+	void (*callback)(struct ia *);
 
 	d_printf(LOG_DEBUG, FNAME, "prefix timeout for %s/%d",
 	    in6addr2str(&sp->prefix.addr, 0), sp->prefix.plen);
